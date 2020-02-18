#include "HgemmMfma.h"

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
static bool g_EnTensileLayout = false;
static bool g_passCpu = false;

static uint32_t g_DataType = 2; // 1=fp32; 2=fp16; 3=bf16
static unsigned int M, N, K;
static unsigned int Padding, StrideA0, StrideB0, StrideD0;
static DataMem<float> * g_DataA, *g_DataB, *g_DataD, *g_DataRef;
static DataMem<float> * g_hDataA, *g_hDataB, *g_hDataD, *g_hDataRef;
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
	uint32_t wave_method = *(uint32_t*)ca->GetOneArg(GEMM_ARG_WV);
	uint32_t mfma_pttn0 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MT0);
	uint32_t mfma_pttn1 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MT1);
	uint32_t depth_u = *(uint32_t*)ca->GetOneArg(GEMM_ARG_DU);
	g_EnTensileLayout = *(uint32_t*)ca->GetOneArg(GEMM_ARG_TENSILE);

	wave_method = 2;
	mfma_pttn0 = 2;
	mfma_pttn1 = 2;
	depth_u = 32;

	// get kernel parameters
	kernelParam.gemmType = g_DataType;
	kernelParam.waveMethod = wave_method;
	kernelParam.enTensileLayout = g_EnTensileLayout;
	kernelParam.M = M; kernelParam.N = N; kernelParam.K = K;
	kernelParam.wave_num_per_simd = 1;
	kernelParam.mfma_pttn_per_wave[0] = mfma_pttn0;
	kernelParam.mfma_pttn_per_wave[1] = mfma_pttn1;
	kernelParam.wave_pttn_per_group[0] = 2; 
	kernelParam.wave_pttn_per_group[1] = 2;
	kernelParam.DepthU = depth_u;
	kernelParam.dbgNum = g_debugDataNum;

	// generate kernel source
	kernelWriter = new GemmMfmaKernelWriter(kernelParam);

	if (g_EnTensileLayout == false)
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
		setParam(k, g_hDataA, g_hDataB, g_hDataD, g_dbg_buff, M, N, K, StrideA0, StrideB0, StrideD0);
	}

	repeatTimes = 1;
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

		g_hDataD->Sync2Hst();
		pf32 = (float*)g_DataD->HstAddr();
		pf16 = (short*)g_hDataD->HstAddr();
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
	g_DataD->LogDev(E_DataFormat::Nomal, 2, 0, 127);

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
	
	//M = 64; N = 64; K = 32; Padding = 0; // 1 cu 0 loop debug
	//M = 4096; N = 4096; K = 256; Padding = 32;
	//M = 128 * 8; N = 128 * 8; K = 512; Padding = 32; // for test
	M = 128 * 8; N = 128 * 8; K = 192; Padding = 32; // for test
	
	//M = 7680; N = 8192; K = 8192; Padding = 32; // for tensile test
	StrideA0 = K + Padding;
	StrideB0 = K + Padding;
	StrideD0 = M + Padding;

	if ((M > 4096) || (K > 512))
		g_passCpu = true;

	uint32_t lds_sz = (MAX_LDS_SIZE / GPR_SZ) * 1;

	g_DataA = newRealData<float>("matrix-a", 2.3f, StrideA0, M);
	g_DataB = newRealData<float>("matrix-b", -2.3f, StrideB0, N);
	g_DataD = newRealData<float>("matrix-d", 0, StrideD0, N);
	g_dbg_buff = newRealData<float>("debug-buffer", 55.55f, lds_sz);
	g_DataRef = newRealData<float>("matrix-ref", -55.55, StrideD0, N);
	g_DataRef->SetMemType(E_MemType::Page);

	g_hDataA = newRealData<float>("matrix-a", 0, StrideA0/2, M);
	g_hDataB = newRealData<float>("matrix-b", 0, StrideB0/2, N);
	g_hDataD = newRealData<float>("matrix-d", 0, StrideD0/2, N);
	g_hDataRef = newRealData<float>("matrix-ref", -55.55f, StrideD0, N);
	g_hDataRef->SetMemType(E_MemType::Page);
	g_dbg_h2f_buff = newRealData<float>("debug-fp16", 55.55f, lds_sz);

	if (g_passCpu)
		return ;

	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		pf32 = (float*)g_DataA->HstAddr();
		pf16 = (short*)g_hDataA->HstAddr();
		for (uint32_t i = 0; i < StrideA0*M; i++)
			pf16[i] = cvtF32toF16(pf32[i]);
		g_hDataA->SetMemType(E_MemType::Page);
		g_hDataA->Sync2Dev();
		g_hDataA->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataB->HstAddr();
		pf16 = (short*)g_hDataB->HstAddr();
		for (uint32_t i = 0; i < StrideB0*N; i++)
			pf16[i] = cvtF32toF16(pf32[i]);
		g_hDataB->SetMemType(E_MemType::Page);
		g_hDataB->Sync2Dev();
		g_hDataB->SetMemType(E_MemType::Dev);
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
		pf16 = (short*)g_hDataRef->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			pf16[i] = cvtF32toF16(pf32[i]);
		g_hDataRef->SetMemType(E_MemType::Page);
		g_hDataRef->Sync2Dev();
		g_hDataRef->SetMemType(E_MemType::Dev);
	}
}

