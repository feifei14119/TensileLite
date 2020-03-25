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
static bool g_cpuVerify = true;
uint32_t enTensileLayout = 0;

static uint32_t g_DataType; // 1=fp32; 2=fp16; 3=bf16
static unsigned int M, N, K;
static unsigned int Padding, StrideA0, StrideB0, StrideD0;
static DataMem<float> * g_DataA, *g_DataB, *g_DataD, *g_DataRef;
static DataMem<float> * g_hfDataA, *g_hfDataB, *g_hfDataD, *g_hfDataRef;
static DataMem<float> * g_bfDataA, *g_bfDataB, *g_bfDataD, *g_bfDataRef;
static DataMem<float> * g_dbg_buff, *g_dbg_buff_16to32; // g_dbg_buff: device debug buffer(fp32 or fp16);
														// g_dbg_buff_16to32: if g_dbg_buff is fp16, cvt it to fp32 to this buffer

/************************************************************************/
/* solution控制                                                          */
/************************************************************************/
E_ReturnState GemmMfmaAsmSolution::generateKernel()
{
	SolutionCtrlBase::generateKernel();

	CmdArgs * ca = CmdArgs::GetCmdArgs();

	enTensileLayout = *(uint32_t*)ca->GetOneArg(GEMM_ARG_TENSILE);
	uint32_t mfma_pttn0 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MT0);
	uint32_t mfma_pttn1 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MT1);
	uint32_t wave_pttn0 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_WT0);
	uint32_t wave_pttn1 = *(uint32_t*)ca->GetOneArg(GEMM_ARG_WT1);
	uint32_t depth_u = *(uint32_t*)ca->GetOneArg(GEMM_ARG_DU);
	uint32_t lds_buffer_num = *(uint32_t*)ca->GetOneArg(GEMM_ARG_BUFFER);


	LOG("lds buffer number = %d", lds_buffer_num);
	LOG("loop unroll = %d", depth_u);
	LOG("mfma pattern per wave  = [%d, %d]", mfma_pttn0, mfma_pttn1);
	LOG("wave pattern per group = [%d, %d]", wave_pttn0, wave_pttn1);

	// get kernel parameters
	if (g_DataType == 1)kernelParam.dataType = E_DataType::Fp32;
	if (g_DataType == 2)kernelParam.dataType = E_DataType::Fp16;
	if (g_DataType == 3)kernelParam.dataType = E_DataType::Bf16;
	kernelParam.enTensileLayout = enTensileLayout;
	kernelParam.M = M; kernelParam.N = N; kernelParam.K = K;
	kernelParam.mfma_pttn_per_wave[0] = mfma_pttn0;
	kernelParam.mfma_pttn_per_wave[1] = mfma_pttn1;
	kernelParam.wave_pttn_per_group[0] = wave_pttn0;
	kernelParam.wave_pttn_per_group[1] = wave_pttn1;
	kernelParam.DepthU = depth_u;
	kernelParam.lds_buffer_num = lds_buffer_num;
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
	T_Dispatch disp = kernelWriter->GetDispatch();
	dim disp_group_num = disp.global_size / disp.group_size;
	std::string kernelName = kernelWriter->KernelName();
	std::string kernelFile = kernelWriter->KernelFile();
	LOG("group_size = [%d, %d, %d].", disp.group_size.x, disp.group_size.y, disp.group_size.z);
	LOG("global_size = [%d, %d, %d].", disp.global_size.x, disp.global_size.y, disp.global_size.z);
	LOG("group_num = [%d, %d, %d].", disp_group_num.x, disp_group_num.y, disp_group_num.z);
	LOG("kernel name: " + kernelName);
	LOG("kernel file: " + kernelFile);

	// build up kernel obj
	GpuRuntimeBase * rt = GpuRuntime::GetInstance();
	GpuKernelBase * k = rt->CreateKernel((char*)kernelFile.c_str(), kernelName.c_str(), E_ProgramType::GAS_FILE);

	if (enTensileLayout == true)
		return E_ReturnState::SUCCESS;

	if (k == nullptr)
		return E_ReturnState::RTN_ERR;
	kernels.push_back(k);

	// set up framework to launch kernel
	dispatches.push_back(disp);
	if (g_DataType == 1)
	{
		setParam(k, g_DataA, g_DataB, g_DataD, g_dbg_buff, M, N, K, StrideA0, StrideB0, StrideD0);
	}
	else if (g_DataType == 2)
	{
		setParam(k, g_hfDataA, g_hfDataB, g_hfDataD, g_dbg_buff, M, N, K, StrideA0, StrideB0, StrideD0);
	}
	else if (g_DataType == 3)
	{
		setParam(k, g_bfDataA, g_bfDataB, g_bfDataD, g_dbg_buff, M, N, K, StrideA0, StrideB0, StrideD0);
	}

	score.Calculation = 2.0 * M*N*K;
	repeatTimes = *(uint32_t*)ca->GetOneArg(GEMM_ARG_LOOP);

	double sclk_mhz = 1289.0;
	uint32_t cu_num = 120;
	if (g_DataType == 1)score.TheoryFlops = sclk_mhz * cu_num * 256;  // fp32
	if (g_DataType == 3)score.TheoryFlops = sclk_mhz * cu_num * 512;  // bf16
	if (g_DataType == 2)score.TheoryFlops = sclk_mhz * cu_num * 1024; // fp16
	score.TheoryFlops = score.TheoryFlops * 1000 * 1000;
	return E_ReturnState::SUCCESS;
}
E_ReturnState GemmMfmaAsmSolution::verifyResult()
{
	if (enTensileLayout == true)
		return E_ReturnState::SUCCESS;
	if (!g_cpuVerify)
		return E_ReturnState::SUCCESS;

	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		g_hfDataD->Sync2Hst();
		pf32 = (float*)g_DataD->HstAddr();
		pf16 = (short*)g_hfDataD->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			pf32[i] = cvtFP16toFP32(pf16[i]);
		g_DataD->SetMemType(E_MemType::Page);
		g_DataD->Sync2Dev();
		g_DataD->SetMemType(E_MemType::Dev);

		g_dbg_buff->Sync2Hst();
		pf32 = (float*)g_dbg_buff_16to32->HstAddr();
		pf16 = (short*)g_dbg_buff->HstAddr();
		for (uint32_t i = 0; i < g_dbg_buff->DataCount(); i++)
			pf32[i] = cvtFP16toFP32(pf16[i]);
		g_dbg_buff_16to32->SetMemType(E_MemType::Page);
		g_dbg_buff_16to32->Sync2Dev();
		g_dbg_buff_16to32->SetMemType(E_MemType::Dev);
	}
	
	if (g_DataType == 3)
	{
		// g_bfDataD -> g_DataD: bf16 -> fp32
		// g_bfDataRef -> g_DataRef: bf16 -> fp32
		// compare g_DataD VS g_DataRef
		float *pf32;
		short *bf16;

		g_bfDataD->Sync2Hst();
		pf32 = (float*)g_DataD->HstAddr();
		bf16 = (short*)g_bfDataD->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			pf32[i] = cvtBF16toFP32(bf16[i]);
		g_DataD->SetMemType(E_MemType::Page);
		g_DataD->Sync2Dev();
		g_DataD->SetMemType(E_MemType::Dev);

		g_dbg_buff->Sync2Hst();
		pf32 = (float*)g_dbg_buff_16to32->HstAddr();
		bf16 = (short*)g_dbg_buff->HstAddr();
		for (uint32_t i = 0; i < g_dbg_buff->DataCount(); i++)
			pf32[i] = cvtBF16toFP32(bf16[i]);
		g_dbg_buff_16to32->SetMemType(E_MemType::Page);
		g_dbg_buff_16to32->Sync2Dev();
		g_dbg_buff_16to32->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataRef->HstAddr();
		bf16 = (short*)g_bfDataRef->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			pf32[i] = cvtBF16toFP32(bf16[i]);
		g_DataRef->SetMemType(E_MemType::Page);
		g_DataRef->Sync2Dev();
		g_DataRef->SetMemType(E_MemType::Dev);
	}

	if (g_DataType == 1)
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
	else if (g_DataType == 2)
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
	else if (g_DataType == 3)
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

	if ((g_DataType == 2) || (g_DataType == 3))
	{
		g_dbg_buff_16to32->Dump(); // dump device debug buffer cvt to fp32
		//g_dbg_buff_16to32->LogDev(E_DataFormat::Nomal, 2, 0, 255);
	}

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
	g_cpuVerify = *(uint32_t*)ca->GetOneArg(GEMM_ARG_VERIFY);


	Padding = 32;
	StrideA0 = K + Padding;
	StrideB0 = K + Padding;
	StrideD0 = M + Padding;

	LOG("data type = %d", g_DataType);
	LOG("gemm size: M,N,K = %d, %d, %d.", M, N, K);
	LOG("stride: A,B,D = %d, %d, %d.", StrideA0, StrideB0, StrideD0);
	
	uint32_t lds_sz = (MAX_LDS_SIZE / GPR_SZ) * 1;

	g_DataA = newRealData<float>("matrix-a", 2.4f, StrideA0, M);
	g_DataB = newRealData<float>("matrix-b", -2.4f, StrideB0, N);
	g_DataD = newRealData<float>("matrix-d", 0, StrideD0, N);
	g_dbg_buff = newRealData<float>("debug-buffer", 55.55f, lds_sz);
	g_DataRef = newRealData<float>("matrix-ref", -55.55, StrideD0, N);
	g_DataRef->SetMemType(E_MemType::Page);
	
	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		g_hfDataA = newRealData<float>("matrix-a", 0, StrideA0 / 2, M);
		g_hfDataB = newRealData<float>("matrix-b", 0, StrideB0 / 2, N);
		g_hfDataD = newRealData<float>("matrix-d", 0, StrideD0 / 2, N);
		g_hfDataRef = newRealData<float>("matrix-ref", -55.55f, StrideD0, N);
		g_hfDataRef->SetMemType(E_MemType::Page);
		g_dbg_buff_16to32 = newRealData<float>("debug-fp16", 55.55f, lds_sz);

		if (!g_cpuVerify)
			return;

		pf32 = (float*)g_DataA->HstAddr();
		pf16 = (short*)g_hfDataA->HstAddr();
		for (uint32_t i = 0; i < StrideA0*M; i++)
			pf16[i] = cvtFP32toFP16(pf32[i]);
		g_hfDataA->SetMemType(E_MemType::Page);
		g_hfDataA->Sync2Dev();
		g_hfDataA->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataB->HstAddr();
		pf16 = (short*)g_hfDataB->HstAddr();
		for (uint32_t i = 0; i < StrideB0*N; i++)
			pf16[i] = cvtFP32toFP16(pf32[i]);
		g_hfDataB->SetMemType(E_MemType::Page);
		g_hfDataB->Sync2Dev();
		g_hfDataB->SetMemType(E_MemType::Dev);
	}

	if (g_DataType == 3)
	{
		float *pf32;
		short *bf16;

		g_bfDataA = newRealData<float>("matrix-a", 0, StrideA0 / 2, M);
		g_bfDataB = newRealData<float>("matrix-b", 0, StrideB0 / 2, N);
		g_bfDataD = newRealData<float>("matrix-d", 0, StrideD0 / 2, N);
		g_bfDataRef = newRealData<float>("matrix-ref", -55.55f, StrideD0, N);
		g_bfDataRef->SetMemType(E_MemType::Page);
		g_dbg_buff_16to32 = newRealData<float>("debug-bf16 to fp32", 55.55f, lds_sz);

		if (!g_cpuVerify)
			return;

		pf32 = (float*)g_DataA->HstAddr();
		bf16 = (short*)g_bfDataA->HstAddr();
		for (uint32_t i = 0; i < StrideA0*M; i++)
			bf16[i] = cvtFP32toBF16(pf32[i]);
		g_bfDataA->SetMemType(E_MemType::Page);
		g_bfDataA->Sync2Dev();
		g_bfDataA->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataB->HstAddr();
		bf16 = (short*)g_bfDataB->HstAddr();
		for (uint32_t i = 0; i < StrideB0*N; i++)
			bf16[i] = cvtFP32toBF16(pf32[i]);
		g_bfDataB->SetMemType(E_MemType::Page);
		g_bfDataB->Sync2Dev();
		g_bfDataB->SetMemType(E_MemType::Dev);
	}
}
void GemmMfmaProblem::cpuCompute()
{
	ProblemCtrlBase::cpuCompute();

	if (!g_cpuVerify)
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
			pf16[i] = cvtFP32toFP16(pf32[i]);
		g_hfDataRef->SetMemType(E_MemType::Page);
		g_hfDataRef->Sync2Dev();
		g_hfDataRef->SetMemType(E_MemType::Dev);
	}

	if (g_DataType == 3)
	{
		float *pf32;
		short *bf16;
		pf32 = (float*)g_DataRef->HstAddr();
		bf16 = (short*)g_bfDataRef->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N; i++)
			bf16[i] = cvtFP32toBF16(pf32[i]);
		g_bfDataRef->SetMemType(E_MemType::Page);
		g_bfDataRef->Sync2Dev();
		g_bfDataRef->SetMemType(E_MemType::Dev);
	}
}


