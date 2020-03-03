#include "gemmMfma.h"

using namespace feifei;

/*
A = M(high) * K(width)
B = N(width) * K(high)
C = M(high) * N(width)
列主序TN
A:	______K_______
	|		  	 | M
	|____________|

B:	______K_______
	|			 | N
	|____________|

C:	______M_______
	|			 | N
	|____________|
*/

static unsigned int g_debugDataNum = 1;
static bool g_passCpu = false;

static uint32_t g_DataType = 1; // 1=fp32; 2=fp16; 3=bf16
static unsigned int M, N, K;
static unsigned int Padding, StrideA0, StrideB0, StrideD0;
static DataMem<float> * g_DataA, *g_DataB, *g_DataD, *g_DataRef;
static DataMem<float> * g_hfDataA, *g_hfDataB, *g_hfDataD, *g_hfDataRef;
static DataMem<float> * g_dbg_buff, *g_dbg_h2f_buff;

static inline unsigned int f32_as_u32(float f) { union { float f; unsigned int u; } v; v.f = f; return v.u; }
static inline float u32_as_f32(unsigned int u) { union { float f; unsigned int u; } v; v.u = u; return v.f; }
static inline int clamp_int(int i, int l, int h) { return std::min(std::max(i, l), h); }
short cvtF32toF16(float f)
{
	uint32_t * p = (uint32_t*)&f;

	unsigned int sig = (*p >> 16) & 0x8000;
	int exp = ((*p >> 23) & 0xff) - 127 + 15;
	unsigned int m = ((*p >> 11) & 0xffe) | ((*p & 0xfff) != 0);
	unsigned int i = 0x7c00 | (m != 0 ? 0x0200 : 0);
	unsigned int n = (exp << 12) | m;

	int b = clamp_int(1 - exp, 0, 13);
	unsigned int d = (0x1000 | m) >> b;
	d |= (d << b) != (0x1000 | m);
	unsigned int v = exp < 1 ? d : n;

	v = (v >> 2) + (((v & 0x7) == 3) | ((v & 0x7) > 5));
	v = exp > 30 ? 0x7c00 : v;
	v = exp == 143 ? i : v;

	short r = sig | v;
	return r;
}
float cvtF16toF32(unsigned short a)
{
	unsigned int u = ((a << 13) + 0x70000000U) & 0x8fffe000U;
	unsigned int v = f32_as_u32(u32_as_f32(u) * pow(1.0, 112)) + 0x38000000U;
	u = (a & 0x7fff) != 0 ? v : u;
	return u32_as_f32(u) * pow(1.0, -112);
}

/************************************************************************/
/* solution控制                                                          */
/************************************************************************/
E_ReturnState GemmMfmaAsmSolution::generateKernel()
{
	SolutionCtrlBase::generateKernel();

	CmdArgs * ca = CmdArgs::GetCmdArgs();

	uint32_t wave_method = 2;
	uint32_t mfma_pttn0 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MT0);
	uint32_t mfma_pttn1 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MT1);
	uint32_t wave_pttn0 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_WT0);
	uint32_t wave_pttn1 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_WT1);
	uint32_t depth_u = *(uint32_t*)ca->GetOneArg(GEMM_ARG_DU);
	uint32_t enTensileLayout = *(uint32_t*)ca->GetOneArg(GEMM_ARG_TENSILE);

	//mfma_pttn0 = 1;
	//mfma_pttn1 = 1;
	//wave_pttn0 = 2;
	//wave_pttn1 = 2;
	//depth_u = 16;
	//enTensileLayout = 0;
	LOG("data type = %d", g_DataType);
	LOG("mfma pattern per wave  = [%d, %d]", mfma_pttn0, mfma_pttn1);
	LOG("wave pattern per group = [%d, %d]", wave_pttn0, wave_pttn1);
	LOG("loop unroll = %d", depth_u);

	// get kernel parameters
	if (g_DataType == 1)kernelParam.dataType = E_DataType::Fp32;
	if (g_DataType == 2)kernelParam.dataType = E_DataType::Fp16;
	if (g_DataType == 3)kernelParam.dataType = E_DataType::Bf16;
	kernelParam.waveMethod = wave_method;
	kernelParam.enTensileLayout = enTensileLayout;
	kernelParam.M = M; kernelParam.N = N; kernelParam.K = K;
	kernelParam.mfma_pttn_per_wave[0] = mfma_pttn0;
	kernelParam.mfma_pttn_per_wave[1] = mfma_pttn1;
	kernelParam.wave_pttn_per_group[0] = wave_pttn0;
	kernelParam.wave_pttn_per_group[1] = wave_pttn1;
	kernelParam.DepthU = depth_u;
	kernelParam.dbgNum = g_debugDataNum;

	// generate kernel source
	kernelWriter = new GemmMfmaKernelWriter(kernelParam);

	if (enTensileLayout == false)
	{
		kernelWriter->SetArg("A", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("B", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("D", sizeof(float*), E_ArgKind::Global);
		kernelWriter->SetArg("dbg_buff", sizeof(float*), E_ArgKind::Global);
		kernelWriter->SetArg("M", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("N", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("K", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideA0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideB0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideD0", sizeof(uint32_t), E_ArgKind::Value);
	}
	ChkErr(kernelWriter->GenKernelString());
	kernelWriter->SaveKernelString2File();

	// get back kernel info
	std::string kernelName = kernelWriter->KernelName();
	std::string kernelFile = kernelWriter->KernelFile();

	// build up kernel obj
	GpuRuntimeBase * rt = GpuRuntime::GetInstance();
	GpuKernelBase * k = rt->CreateKernel((char*)kernelFile.c_str(), kernelName.c_str(), E_ProgramType::GAS_FILE);

	if (k == nullptr)
		return E_ReturnState::RTN_ERR;
	kernels.push_back(k);

	// set up framework to launch kernel
	T_Dispatch disp = kernelWriter->GetDispatch();
	dispatches.push_back(disp);
	if (g_DataType == 1)
	{
		setParam(k, g_DataA, g_DataB, g_DataD, g_dbg_buff, M, N, K, StrideA0, StrideB0, StrideD0);
	}
	else if (g_DataType == 2)
	{
		setParam(k, g_hfDataA, g_hfDataB, g_hfDataD, g_dbg_buff, M, N, K, StrideA0, StrideB0, StrideD0);
	}

	repeatTimes = 1;
	if (g_passCpu == true)	repeatTimes = 10;
	score.Calculation = 2.0 * M*N*K;

	if (g_DataType == 1)score.TheoryFlops = 1.0 * 758 * 256 * 120; // fp32
	if (g_DataType == 3)score.TheoryFlops = 1.0 * 758 * 512 * 120; // bf16
	if (g_DataType == 2)score.TheoryFlops = 1.0 * 758 * 1024 * 120; // fp16
	return E_ReturnState::SUCCESS;
}
E_ReturnState GemmMfmaAsmSolution::verifyResult()
{
	if (g_passCpu)
		return E_ReturnState::SUCCESS;

	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		g_hfDataD->Sync2Hst();
		pf32 = (float*)g_DataD->HstAddr();
		pf16 = (short*)g_hfDataD->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			pf32[i] = cvtF16toF32(pf16[i]);
		g_DataD->SetMemType(E_MemType::Page);
		g_DataD->Sync2Dev();
		g_DataD->SetMemType(E_MemType::Dev);

		g_dbg_buff->Sync2Hst();
		pf32 = (float*)g_dbg_h2f_buff->HstAddr();
		pf16 = (short*)g_dbg_buff->HstAddr();
		for (uint32_t i = 0; i < g_dbg_buff->DataCount(); i++)
			pf32[i] = cvtF16toF32(pf16[i]);
		g_dbg_h2f_buff->SetMemType(E_MemType::Page);
		g_dbg_h2f_buff->Sync2Dev();
		g_dbg_h2f_buff->SetMemType(E_MemType::Dev);
	}
	
	//CompareDataMem(g_DataD, g_DataRef);
	{
		g_DataD->Sync2Hst();
		float *pgpu;
		float *pcpu;
		pgpu = (float*)g_DataD->HstAddr();
		pcpu = (float*)g_DataRef->HstAddr();

		bool nopass = false;
		for (uint32_t n = 0; n < N; n++)
		{
			for (uint32_t m = 0; m < M; m++)
			{ 
				uint32_t idx = n * StrideD0 + m;
				float diff = pgpu[idx] - pcpu[idx];
				if ((diff > 0.1f) || (diff < -0.1f))
				{
					nopass = true;
					WARN("Verify Failed! [%d]:gpu=%.2f,cpu=%.2f", idx, pgpu[idx], pcpu[idx]);
					break;
				}
			}

			if (nopass == true)
				break;
		}
		if (nopass == false)
		{
			LOG("Verify Success.");
		}
	}

	g_DataRef->Dump();
	//g_DataRef->LogHst(E_DataFormat::Nomal, 2, 0, 127);
	
	g_DataD->Dump();
	//g_DataD->LogDev(E_DataFormat::Nomal, 2, 0, 127);

	g_dbg_buff->Dump();
	//g_dbg_buff->LogDev(E_DataFormat::Nomal, 2, 0, 255);

	g_dbg_h2f_buff->Dump();
	//g_dbg_h2f_buff->LogDev(E_DataFormat::Nomal, 2, 0, 255);

	return E_ReturnState::SUCCESS;
}

/************************************************************************/
/* solver 控制															*/
/************************************************************************/
void GemmMfmaSolver::generateSolver()
{
	solutions.push_back(new GemmMfmaAsmSolution());
}

/************************************************************************/
/* 问题控制                                                             */
/************************************************************************/
void GemmMfmaProblem::initDataMem()
{
	ProblemCtrlBase::initDataMem();

	CmdArgs * ca = CmdArgs::GetCmdArgs();
	g_DataType = *(uint32_t*)ca->GetOneArg(GEMM_ARG_TYPE);
	M = *(uint32_t*)ca->GetOneArg(GEMM_ARG_M);
	N = *(uint32_t*)ca->GetOneArg(GEMM_ARG_N);
	K = *(uint32_t*)ca->GetOneArg(GEMM_ARG_K);

	//M = 128 * 6; N = 128 * 5; K = 256; Padding = 32;
	//M = 4096; N = 512; K = 4096;
	Padding = 32;
	StrideA0 = K + Padding;
	StrideB0 = K + Padding;
	StrideD0 = M + Padding;

	LOG("gemm size: M,N,K = %d, %d, %d.", M, N, K);

	if ((M > 4096) || (K > 512))
		g_passCpu = true;

	uint32_t lds_sz = (MAX_LDS_SIZE / GPR_SZ) * 1;

	g_DataA = newRealData<float>("matrix-a", 2.3f, StrideA0, M);
	g_DataB = newRealData<float>("matrix-b", -2.3f, StrideB0, N);
	g_DataD = newRealData<float>("matrix-d", 0, StrideD0, N);
	g_dbg_buff = newRealData<float>("debug-buffer", 55.55f, lds_sz);
	g_DataRef = newRealData<float>("matrix-ref", -55.55, StrideD0, N);
	g_DataRef->SetMemType(E_MemType::Page);

	g_hfDataA = newRealData<float>("matrix-a", 0, StrideA0/2, M);
	g_hfDataB = newRealData<float>("matrix-b", 0, StrideB0/2, N);
	g_hfDataD = newRealData<float>("matrix-d", 0, StrideD0/2, N);
	g_hfDataRef = newRealData<float>("matrix-ref", -55.55f, StrideD0, N);
	g_hfDataRef->SetMemType(E_MemType::Page);
	g_dbg_h2f_buff = newRealData<float>("debug-fp16", 55.55f, lds_sz);

	if (g_passCpu)
		return ;

	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		pf32 = (float*)g_DataA->HstAddr();
		pf16 = (short*)g_hfDataA->HstAddr();
		for (uint32_t i = 0; i < StrideA0*M; i++)
			pf16[i] = cvtF32toF16(pf32[i]);
		g_hfDataA->SetMemType(E_MemType::Page);
		g_hfDataA->Sync2Dev();
		g_hfDataA->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataB->HstAddr();
		pf16 = (short*)g_hfDataB->HstAddr();
		for (uint32_t i = 0; i < StrideB0*N; i++)
			pf16[i] = cvtF32toF16(pf32[i]);
		g_hfDataB->SetMemType(E_MemType::Page);
		g_hfDataB->Sync2Dev();
		g_hfDataB->SetMemType(E_MemType::Dev);
	}
}
void GemmMfmaProblem::cpuCompute()
{
	ProblemCtrlBase::cpuCompute();

	if (g_passCpu)
		return;

	float * h_a = (float*)g_DataA->HstAddr();
	float * h_b = (float*)g_DataB->HstAddr();
	float * d_ref = (float*)g_DataRef->HstAddr();

	for (int n = 0; n < N; n++)
	{
		for (int m = 0; m < M; m++)
		{
			float t_sum = 0.0f;
			for (int k = 0; k < K; k++)
			{
				uint32_t idx_a = StrideA0 * m + k;
				uint32_t idx_b = StrideB0 * n + k;
				t_sum += h_a[idx_a] * h_b[idx_b];
			}

			uint32_t idx_d = StrideD0 * n + m;
			d_ref[idx_d] = t_sum;
		}
	}

	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;
		pf32 = (float*)g_DataRef->HstAddr();
		pf16 = (short*)g_hfDataRef->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			pf16[i] = cvtF32toF16(pf32[i]);
		g_hfDataRef->SetMemType(E_MemType::Page);
		g_hfDataRef->Sync2Dev();
		g_hfDataRef->SetMemType(E_MemType::Dev);
	}
}

