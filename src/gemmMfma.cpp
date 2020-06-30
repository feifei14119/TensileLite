#include "gemmMfma.h"

using namespace feifei;

static bool g_cpuVerify = true;
static bool g_dataInit = false;
uint32_t enTensileLayout = 0;

static uint32_t g_DataType; // 1=fp32; 2=fp16; 3=bf16
static unsigned int M, N, K, Batch, Padding;
static float Alpha, Beta;
static unsigned int StrideA0, StrideB0, StrideC0, StrideD0;
static unsigned int BatchStrideA, BatchStrideB, BatchStrideC, BatchStrideD;
static DataMem<float> * g_Fp32CpuRsltRef;
static DataMem<float> * g_DataA, *g_DataB, *g_DataC, *g_DataD;
static DataMem<float> * g_hfDataA, *g_hfDataB, *g_hfDataC, *g_hfDataD;
static DataMem<float> * g_bfDataA, *g_bfDataB, *g_bfDataC, *g_bfDataD;

// =======================================================================
void GemmMfmaSolver::generateSolver()
{
	solutions.push_back(new GemmMfmaAsmSolution());
}
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
	uint32_t mfma_mn = *(uint32_t*)ca->GetOneArg(GEMM_ARG_MFMA_MN);

	/*enTensileLayout = 0;
	mfma_pttn0 = 1;
	mfma_pttn1 = 2;
	wave_pttn0 = 2;
	wave_pttn1 = 2;
	mfma_mn = 32;
	depth_u = 32;
	lds_buffer_num = 2;*/

	LOG("lds buffer number = %d", lds_buffer_num);
	LOG("loop unroll = %d", depth_u);
	LOG("mfma mn = %d", mfma_mn);
	LOG("mfma pattern per wave  = [%d, %d]", mfma_pttn0, mfma_pttn1);
	LOG("wave pattern per group = [%d, %d]", wave_pttn0, wave_pttn1);
	LOG("MT = [%d, %d]", mfma_mn * mfma_pttn0 * wave_pttn0, mfma_mn * mfma_pttn1 * wave_pttn1);

	if (g_DataType == 1)kernelParam.DataType = E_DataType::Fp32;
	if (g_DataType == 2)kernelParam.DataType = E_DataType::Fp16;
	if (g_DataType == 3)kernelParam.DataType = E_DataType::Bf16;
	kernelParam.enTensileLayout = enTensileLayout;
	kernelParam.M = M; kernelParam.N = N; kernelParam.K = K; kernelParam.Batch = Batch;
	kernelParam.mfma_pttn_per_wave[0] = mfma_pttn0;
	kernelParam.mfma_pttn_per_wave[1] = mfma_pttn1;
	kernelParam.wave_pttn_per_group[0] = wave_pttn0;
	kernelParam.wave_pttn_per_group[1] = wave_pttn1;
	kernelParam.mfma_mn = mfma_mn;
	kernelParam.loop_unroll = depth_u;
	kernelParam.lds_buffer_num = lds_buffer_num;

	kernelWriter = new GemmMfmaKernelWriter(kernelParam);

	if (enTensileLayout == true)
	{
		kernelWriter->SetArg("sizeC", sizeof(uint64_t), E_ArgKind::Value);
		kernelWriter->SetArg("sizeA", sizeof(uint64_t), E_ArgKind::Value);
		kernelWriter->SetArg("sizeB", sizeof(uint64_t), E_ArgKind::Value);
		kernelWriter->SetArg("D", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("C", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("A", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("B", sizeof(float*), E_ArgKind::Global);
		kernelWriter->SetArg("alpha", sizeof(float), E_ArgKind::Value);
		kernelWriter->SetArg("beta", sizeof(float), E_ArgKind::Value);
		kernelWriter->SetArg("strideD0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("strideD1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideC0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideC1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideA0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideA1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideB0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideB1", sizeof(uint32_t), E_ArgKind::Value);

		kernelWriter->SetArg("SizesFree0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("SizesFree1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("SizesFree2", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("SizesSum0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("OrigStaggerUIter", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("NumWorkGroups0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("NumWorkGroups1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("MagicNumberProblemNumGroupTiles0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("GridNumWorkGroups0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("NumFullBlocks", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("WgmRemainder1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("MagicNumberWgmRemainder1", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("padding", sizeof(uint32_t), E_ArgKind::Value);
	}
	else
	{
		kernelWriter->SetArg("D", sizeof(float*), E_ArgKind::Global);
		kernelWriter->SetArg("C", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("A", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("B", sizeof(float*), E_ArgKind::Global, true);
		kernelWriter->SetArg("Alpha", sizeof(float), E_ArgKind::Value);
		kernelWriter->SetArg("Beta", sizeof(float), E_ArgKind::Value);
		kernelWriter->SetArg("StrideD0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("BatchStrideD", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideC0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("BatchStrideC", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideA0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("BatchStrideA", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("StrideB0", sizeof(uint32_t), E_ArgKind::Value);
		kernelWriter->SetArg("BatchStrideB", sizeof(uint32_t), E_ArgKind::Value);
	}
	ChkErr(kernelWriter->GenKernelString());
	kernelWriter->SaveKernelString2File();

	T_Dispatch disp = kernelWriter->GetDispatch();
	dim disp_group_num = disp.global_size / disp.group_size;
	std::string kernelName = kernelWriter->KernelName();
	std::string kernelFile = kernelWriter->KernelFile();
	LOG("group size   = [%d, %d, %d].", disp.group_size.x, disp.group_size.y, disp.group_size.z);
	LOG("group number = [%d, %d, %d].", disp_group_num.x, disp_group_num.y, disp_group_num.z);
	LOG("global size  = [%d, %d, %d].", disp.global_size.x, disp.global_size.y, disp.global_size.z);
	LOG("kernel name: " + kernelName);
	LOG("kernel file: " + kernelFile);

	GpuRuntimeBase * rt = GpuRuntime::GetInstance();
	GpuKernelBase * k = rt->CreateKernel((char*)kernelFile.c_str(), kernelName.c_str(), E_ProgramType::GAS_FILE);

	if (enTensileLayout == true)
		return E_ReturnState::SUCCESS;

	if (k == nullptr)
		return E_ReturnState::RTN_ERR;
	kernels.push_back(k);

	dispatches.push_back(disp);
	if (g_DataType == 1)
	{
		setParam(k,
			g_DataD, g_DataC, g_DataA, g_DataB,
			Alpha, Beta,
			StrideD0, BatchStrideD, StrideC0, BatchStrideC,
			StrideA0, BatchStrideA, StrideB0, BatchStrideB);
	}
	else if (g_DataType == 2)
	{
		setParam(k,
			g_hfDataD, g_hfDataC, g_hfDataA, g_hfDataB, 
			Alpha, Beta,
			StrideD0, BatchStrideD, StrideC0, BatchStrideC,
			StrideA0, BatchStrideA, StrideB0, BatchStrideB);
	}
	else if (g_DataType == 3)
	{
		setParam(k,
			g_bfDataD, g_bfDataC, g_bfDataA, g_bfDataB, 
			Alpha, Beta,
			StrideD0, BatchStrideD, StrideC0, BatchStrideC,
			StrideA0, BatchStrideA, StrideB0, BatchStrideB);
	}

	score.Calculation = 2.0 * M*N*K*Batch;
	repeatTimes = *(uint32_t*)ca->GetOneArg(GEMM_ARG_LOOP);
	//repeatTimes = 5;

	double sclk_mhz;
	uint32_t cu_num;
	cu_num = 120;
	sclk_mhz = 1289.0;
	sclk_mhz = 1087.0;
	if (g_DataType == 1)score.TheoryFlops = sclk_mhz * cu_num * 256;  // fp32
	if (g_DataType == 2)score.TheoryFlops = sclk_mhz * cu_num * 1024; // fp16
	if (g_DataType == 3)score.TheoryFlops = sclk_mhz * cu_num * 512;  // bf16
	score.TheoryFlops = score.TheoryFlops * 1000 * 1000;
	
	return E_ReturnState::SUCCESS;
}

void GemmMfmaProblem::initDataMem()
{
	ProblemCtrlBase::initDataMem();

	CmdArgs * ca = CmdArgs::GetCmdArgs();
	g_DataType = *(uint32_t*)ca->GetOneArg(GEMM_ARG_TYPE);
	M = *(uint32_t*)ca->GetOneArg(GEMM_ARG_M);
	N = *(uint32_t*)ca->GetOneArg(GEMM_ARG_N);
	K = *(uint32_t*)ca->GetOneArg(GEMM_ARG_K);
	g_cpuVerify = *(uint32_t*)ca->GetOneArg(GEMM_ARG_VERIFY);
	Padding = *(uint32_t*)ca->GetOneArg(GEMM_ARG_PAD);
	
	/*g_DataType = 1;
	M = 960;
	N = 1024;
	K = 1024;
	Batch = 1;
	Padding = 32;
	g_cpuVerify = true;*/

	if (g_cpuVerify == true)
		g_dataInit = true;

	StrideA0 = K + Padding;
	StrideB0 = K + Padding;
	StrideC0 = M + Padding;
	StrideD0 = M + Padding;
	BatchStrideA = StrideA0 * M;
	BatchStrideB = StrideB0 * N;
	BatchStrideC = StrideC0 * N;
	BatchStrideD = StrideD0 * N;

	if (g_DataType == 1) LOG("data type = FP32");
	if (g_DataType == 2) LOG("data type = FP16");
	if (g_DataType == 3) LOG("data type = BF16");
	LOG("gemm size: M,N,K,Batch = %d, %d, %d, %d.", M, N, K, Batch);
	LOG("row   stride: A,B,C,D = %d, %d, %d.", StrideA0, StrideB0, StrideC0, StrideD0);
	LOG("batch stride: A,B,C,D = %d, %d, %d.", BatchStrideA, BatchStrideB, BatchStrideC, BatchStrideD);
	LOG("padding = %d(elements) = %d(DWORD).", Padding, (g_DataType == 1) ? Padding : Padding / 2);
	
	uint32_t lds_sz = (MAX_LDS_SIZE / GPR_SZ) * 1;

	Alpha = 0.314f;
	Beta = 0.2718f;
	g_DataA = newRealData<float>("matrix-a", 2.4f*(int)g_dataInit, StrideA0, M*Batch);
	g_DataB = newRealData<float>("matrix-b", -2.4f*(int)g_dataInit, StrideB0, N*Batch);
	g_DataC = newRealData<float>("matrix-c", 2.4f*(int)g_dataInit, StrideC0, N*Batch);
	g_DataD = newRealData<float>("matrix-d", 0.5f*(int)g_dataInit, StrideD0, N*Batch);
	g_Fp32CpuRsltRef = newRealData<float>("matrix-ref", -55.55f*(int)g_dataInit, StrideD0, N*Batch);
	g_Fp32CpuRsltRef->SetMemType(E_MemType::Page);
	
	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		g_hfDataA = newRealData<float>("matrix-a", 0.5f*(int)g_dataInit, StrideA0 / 2, M*Batch);
		g_hfDataB = newRealData<float>("matrix-b", 0.5f*(int)g_dataInit, StrideB0 / 2, N*Batch);
		g_hfDataC = newRealData<float>("matrix-c", 0.5f*(int)g_dataInit, StrideC0 / 2, N*Batch);
		g_hfDataD = newRealData<float>("matrix-d", 0.5f*(int)g_dataInit, StrideD0 / 2, N*Batch);

		if (!g_cpuVerify)
			return;

		pf32 = (float*)g_DataA->HstAddr();
		pf16 = (short*)g_hfDataA->HstAddr();
		for (uint32_t i = 0; i < StrideA0*M*Batch; i++)
			pf16[i] = cvtFP32toFP16(pf32[i]);
		g_hfDataA->SetMemType(E_MemType::Page);
		g_hfDataA->Sync2Dev();
		g_hfDataA->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataB->HstAddr();
		pf16 = (short*)g_hfDataB->HstAddr();
		for (uint32_t i = 0; i < StrideB0*N*Batch; i++)
			pf16[i] = cvtFP32toFP16(pf32[i]);
		g_hfDataB->SetMemType(E_MemType::Page);
		g_hfDataB->Sync2Dev();
		g_hfDataB->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataC->HstAddr();
		pf16 = (short*)g_hfDataC->HstAddr();
		for (uint32_t i = 0; i < StrideC0*N*Batch; i++)
			pf16[i] = cvtFP32toFP16(pf32[i]);
		g_hfDataC->SetMemType(E_MemType::Page);
		g_hfDataC->Sync2Dev();
		g_hfDataC->SetMemType(E_MemType::Dev);
	}
	if (g_DataType == 3)
	{
		float *pf32;
		short *bf16;

		g_bfDataA = newRealData<float>("matrix-a", 0.5f*(int)g_dataInit, StrideA0 / 2, M*Batch);
		g_bfDataB = newRealData<float>("matrix-b", 0.5f*(int)g_dataInit, StrideB0 / 2, N*Batch);
		g_bfDataC = newRealData<float>("matrix-c", 0.5f*(int)g_dataInit, StrideC0 / 2, N*Batch);
		g_bfDataD = newRealData<float>("matrix-d", 0.5f*(int)g_dataInit, StrideD0 / 2, N*Batch);

		if (!g_cpuVerify)
			return;

		pf32 = (float*)g_DataA->HstAddr();
		bf16 = (short*)g_bfDataA->HstAddr();
		for (uint32_t i = 0; i < StrideA0*M*Batch; i++)
			bf16[i] = cvtFP32toBF16(pf32[i]);
		g_bfDataA->SetMemType(E_MemType::Page);
		g_bfDataA->Sync2Dev();
		g_bfDataA->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataB->HstAddr();
		bf16 = (short*)g_bfDataB->HstAddr();
		for (uint32_t i = 0; i < StrideB0*N*Batch; i++)
			bf16[i] = cvtFP32toBF16(pf32[i]);
		g_bfDataB->SetMemType(E_MemType::Page);
		g_bfDataB->Sync2Dev();
		g_bfDataB->SetMemType(E_MemType::Dev);

		pf32 = (float*)g_DataC->HstAddr();
		bf16 = (short*)g_bfDataC->HstAddr();
		for (uint32_t i = 0; i < StrideC0*N*Batch; i++)
			bf16[i] = cvtFP32toBF16(pf32[i]);
		g_bfDataC->SetMemType(E_MemType::Page);
		g_bfDataC->Sync2Dev();
		g_bfDataC->SetMemType(E_MemType::Dev);
	}
}
void GemmMfmaProblem::cpuCompute()
{
	ProblemCtrlBase::cpuCompute();

	if (!g_cpuVerify)
		return;

	float * h_a = (float*)g_DataA->HstAddr();
	float * h_b = (float*)g_DataB->HstAddr();
	float * h_c = (float*)g_DataC->HstAddr();
	float * d_ref = (float*)g_Fp32CpuRsltRef->HstAddr();

	for (int b = 0; b < Batch; b++)
	{
		for (int n = 0; n < N; n++)
		{
			for (int m = 0; m < M; m++)
			{
				float t_sum = 0.0f;
				for (int k = 0; k < K; k++)
				{
					uint32_t idx_a = BatchStrideA * b + StrideA0 * m + k;
					uint32_t idx_b = BatchStrideB * b + StrideB0 * n + k;
					t_sum += h_a[idx_a] * h_b[idx_b];
				}

				uint32_t idx_c = BatchStrideC * b + StrideC0 * n + m;
				t_sum = Alpha * t_sum + Beta * h_c[idx_c];

				uint32_t idx_d = BatchStrideD * b + StrideD0 * n + m;
				d_ref[idx_d] = t_sum;
			}
		}
	}
}
E_ReturnState GemmMfmaAsmSolution::verifyResult()
{
	if (enTensileLayout == true)
		return E_ReturnState::SUCCESS;
	if (!g_cpuVerify)
		return E_ReturnState::SUCCESS;

	// ---------------------------------------------------------------------
	if (g_DataType == 2)
	{
		float *pf32;
		short *pf16;

		g_hfDataD->Sync2Hst();
		pf32 = (float*)g_DataD->HstAddr();
		pf16 = (short*)g_hfDataD->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N*Batch; i++)
			pf32[i] = cvtFP16toFP32(pf16[i]);
		g_DataD->SetMemType(E_MemType::Page);
		g_DataD->Sync2Dev();
		g_DataD->SetMemType(E_MemType::Dev);
	}
	if (g_DataType == 3)
	{
		float *pf32;
		short *bf16;

		g_bfDataD->Sync2Hst();
		pf32 = (float*)g_DataD->HstAddr();
		bf16 = (short*)g_bfDataD->HstAddr();
		for (uint32_t i = 0; i < StrideD0*N*Batch; i++)
			pf32[i] = cvtBF16toFP32(bf16[i]);
		g_DataD->SetMemType(E_MemType::Page);
		g_DataD->Sync2Dev();
		g_DataD->SetMemType(E_MemType::Dev);
	}

	// ---------------------------------------------------------------------
	g_DataD->Sync2Hst();
	float *pgpu;
	float *pcpu;
	pgpu = (float*)g_DataD->HstAddr();
	pcpu = (float*)g_Fp32CpuRsltRef->HstAddr();

	bool nopass = false;
	for (uint32_t b = 0; b < Batch; b++)
	{
		for (uint32_t n = 0; n < N; n++)
		{
			for (uint32_t m = 0; m < M; m++)
			{
				uint32_t idx = b * BatchStrideD + n * StrideD0 + m;
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
		if (nopass == true)
			break;
	}
	if (nopass == false)
	{
		LOG("Verify Success.");
	}
	
	return E_ReturnState::SUCCESS;
}
