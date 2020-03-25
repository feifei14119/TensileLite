#pragma once

#include "../inc/KernelWriter.h"

using namespace feifei;

#define BF16_SZ	(2)
#define FP16_SZ	(2)
#define FP32_SZ	(4)
typedef struct T_GemmMfmaKernelParamType
{
	E_DataType dataType;
	bool enTensileLayout;

	uint32_t M, N, K;
	uint32_t mfma_pttn_per_wave[2];
	uint32_t wave_pttn_per_group[2];
	uint32_t DepthU;	// loop unroll
	uint32_t lds_buffer_num;
	uint32_t dbgNum;
}T_GemmMfmaKernelParam;

class GemmMfmaKernelWriter :public KernelWriter
{
public:
	GemmMfmaKernelWriter(T_GemmMfmaKernelParam kernelParam) :KernelWriter()
	{
		k_param = kernelParam;
		dbg_buff_len = k_param.dbgNum;

		switch (k_param.dataType)
		{
		case E_DataType::Fp32:
			elem_sz = FP32_SZ;
			mfma_k = 2;
			mfma_inst = "v_mfma_f32_32x32x2f32";
			kernelName = "sgemm_col_TN_MT";
			break;
		case E_DataType::Fp16:
			elem_sz = FP16_SZ;
			mfma_k = 8;
			mfma_inst = "v_mfma_f32_32x32x8f16";
			kernelName = "hgemm_col_TN_MT";
			break;
		case E_DataType::Bf16:
			elem_sz = BF16_SZ;
			mfma_k = 4;
			mfma_inst = "v_mfma_f32_32x32x4bf16";
			kernelName = "bfgemm_col_TN_MT";
			break;
		default:
			elem_sz = FP32_SZ;
			mfma_k = 2;
			mfma_inst = "v_mfma_f32_32x32x2f32";
			kernelName = "sgemm_col_TN_MT";
			break;
		}

		mfma_pttn0_per_wv = k_param.mfma_pttn_per_wave[0];
		mfma_pttn1_per_wv = k_param.mfma_pttn_per_wave[1];
		wv_pttn0_per_grp = k_param.wave_pttn_per_group[0];
		wv_pttn1_per_grp = k_param.wave_pttn_per_group[1];
	}

protected:
	T_GemmMfmaKernelParam k_param;

#pragma  region Variable
	bool en_mfma;
	uint32_t elem_sz; // 每个元素多少BYTE
	uint32_t lds_pad_byte = 8; // LDS每行pad字节数
	int32_t fetch_glb_waitcnt = -1;
	int32_t read_lds_waitcnt = -1;

	bool en_dirct_glb_to_lds = true;
	uint32_t glb_load_instr_dw_sz; // global load size(DWORD)
	uint32_t glb_store_instr_dw_sz; // global stroe size(DWORD)
	uint32_t lds_wr_instr_dw_sz; // lds write size(DWORD)
	uint32_t lds_rd_instr_dw_sz; // lds read size(DWORD)
	uint32_t glb_load_instr_sz; // global load/stroe size(Byte)
	uint32_t glb_store_instr_sz; // global load/stroe size(Byte)
	uint32_t lds_wr_instr_sz; // lds read/write size(Byte)
	uint32_t lds_rd_instr_sz; // lds read/write size(Byte)

	std::string mfma_inst;
	uint32_t mfma_m = 32;
	uint32_t mfma_n = 32;
	uint32_t mfma_k;
	uint32_t mfma_b = 1; // 目前不支持多block的mfma指令
	uint32_t mfma_t = 4; // mfma结果每几行一组存放(目前恒为4)
	uint32_t mfma_dgpr_per_mfma; // 每个thread结果寄存器的个数(可以从手册查到,也可以计算出)
	uint32_t mfma_agpr_per_mfma; // 每个thread源寄存器的个数(可以从手册查到,也可以计算出)
	uint32_t mfma_bgpr_per_mfma; // 每个thread源寄存器的个数(可以从手册查到,也可以计算出)

	// -----------------------------------------------------------------------
	T_Var v_tmp1, v_tmp2;
	T_Var s_tmp1, s_tmp2;
	T_Var v_tid_in_wave;
	T_Var v_wvid_in_grp;
	T_Var s_wvid_in_grp;

	// -----------------------------------------------------------------------
	// config
	// -----------------------------------------------------------------------
	uint32_t wave_num_per_simd;
	uint32_t mfma_pttn0_per_wv;	// 每个wave在MT0方向负责做的mfma次数
	uint32_t mfma_pttn1_per_wv;	// 每个wave在MT1方向负责做的mfma次数
	uint32_t wv_pttn0_per_grp;	// 每个group内wave的排布,每个group内MT0方向wave个数
	uint32_t wv_pttn1_per_grp;	// 每个group内wave的排布,每个group内MT1方向wave个数

	// -----------------------------------------------------------------------
	// workload
	// -----------------------------------------------------------------------
	uint32_t math_wave_num_per_grp; // 每个group总共的math wave数

	// -----------------------------------------------------------------------
	// matrix a
	// -----------------------------------------------------------------------
	T_Var s_a_dscp;
	T_Var v_a_fetch_offset;
	T_Var v_a_fetch_offset_ping;
	T_Var v_a_fetch_offset_pang;
	T_Var s_a_lds_write;
	T_Var s_a_lds_write_0;
	T_Var s_a_lds_write_1;
	T_Var s_a_lds_write_2;
	T_Var v_a_lds_read;
	T_Var v_a_lds_read_0;
	T_Var v_a_lds_read_1;
	T_Var v_a_lds_read_2;
	uint32_t a_fetch_ele_num0_per_wv;	// global->lds:每个wave每次读取的宽度.单位为元素
	uint32_t a_fetch_ele_num1_per_wv;	// global->lds:每个wave每次读取的高度.单位为元素
	uint32_t a_fetch_wave_shape0;		// global->lds:每个wave的宽度.宽度单位为thread
	uint32_t a_fetch_wave_shape1;		// global->lds:每个wave的高度.宽度单位为thread
	uint32_t a_fetch_times;				// global->lds:每个thread需要做多少次fetch
	uint32_t a_lds_sz_per_wv_per_time;	// 一个wave一次fetch写入的LDS大小(Byte)
	uint32_t a_lds_sz_per_wv;			// 一个wave总共fetch写入的LDS大小(Byte)
	uint32_t a_lds_sz_per_grp;			// 一个group总共fetch写入的LDS大小(Byte)
	uint32_t a_lds_read_step_1;			// 不切换tile时,读取下一个所需的步进(K方向步进mfma_k个元素)
	uint32_t a_lds_read_step_2;			// 切换tile时,读取下一个所需的步进

	// -----------------------------------------------------------------------
	// matrix b
	// -----------------------------------------------------------------------
	T_Var s_b_dscp;
	T_Var v_b_fetch_offset;
	T_Var v_b_fetch_offset_ping;
	T_Var v_b_fetch_offset_pang;
	T_Var s_b_lds_write;
	T_Var s_b_lds_write_0;
	T_Var s_b_lds_write_1;
	T_Var s_b_lds_write_2;
	T_Var v_b_lds_read;
	T_Var v_b_lds_read_0;
	T_Var v_b_lds_read_1;
	T_Var v_b_lds_read_2;
	uint32_t b_lds_read_step_1;
	uint32_t b_lds_read_step_2;
	uint32_t b_fetch_ele_num0_per_wv;
	uint32_t b_fetch_ele_num1_per_wv;
	uint32_t b_fetch_wave_shape0;
	uint32_t b_fetch_wave_shape1;
	uint32_t b_fetch_times;
	uint32_t b_lds_sz_per_wv_per_time;
	uint32_t b_lds_sz_per_wv;
	uint32_t b_lds_sz_per_grp;

	// -----------------------------------------------------------------------
	// mfma
	// -----------------------------------------------------------------------
	T_Var v_mfma_a;
	T_Var v_mfma_a_ping, v_mfma_a_pang;
	T_Var v_mfma_b;
	T_Var v_mfma_b_ping, v_mfma_b_pang;
	T_Var acc_mfma_d;
	uint32_t mfma_blk0_per_grp; // 每个group在MT0方向需要mfma的次数
	uint32_t mfma_blk1_per_grp; // 每个group在MT1方向需要mfma的次数
	uint32_t mfma_k_times_per_wv; // 每个wave在k方向需要的次数(mfma结果累加)
	uint32_t mfma_blk_per_wv;	// 每个wave在MT0*MT1方向需要的mfma次数和(mfma结果不累加)
	uint32_t mfma_sz0_per_wv;	// 每个wave得到的MT0方向的结果个数(element)
	uint32_t mfma_sz1_per_wv;	// 每个wave得到的MT1方向的结果个数(element)
	uint32_t elem_num0_per_grp;	// 每个group得到的MT0方向的结果个数(element) = MT0
	uint32_t elem_num1_per_grp;	// 每个group得到的MT1方向的结果个数(element) = MT1
	uint32_t a_mfma_times; // 一次fetch可以做mfma的总次数
	uint32_t b_mfma_times; // 一次fetch可以做mfma的总次数

	// -----------------------------------------------------------------------
	// matrix d
	// -----------------------------------------------------------------------
	T_Var s_d_dscp;
	T_Var v_rslt_d;
	T_Var v_d_store_offset;
	uint32_t d_glb_step1;
	uint32_t d_glb_step2;
	uint32_t mfma_dgpr_tt_sz;

	T_Var lds_buffer_cnt;

	uint32_t k_arg_pA;
	uint32_t k_arg_pB;
	uint32_t k_arg_pD;
	uint32_t k_arg_pDbg;
	uint32_t k_arg_M;
	uint32_t k_arg_N;
	uint32_t k_arg_K;
	uint32_t k_arg_StrideA0;
	uint32_t k_arg_StrideB0;
	uint32_t k_arg_StrideD0;

	T_Var s_args_tmp;

#pragma endregion

	E_ReturnState writeProgramDetail()
	{
		if (k_param.enTensileLayout == true)
		{
			uint32_t bias = 0;
			T_Var s_arg;

			s_args.clear();
		/*0*/	s_arg = newSgpr("SzC", 2, 2);	/*s_load_dword(2, s_arg, s_argsAddr, 8 * 0 + 4 * 0);		*/s_args.push_back(s_arg);
		/*1*/	s_arg = newSgpr("SzA", 2, 2);	/*s_load_dword(2, s_arg, s_argsAddr, 8 * 1 + 4 * 0);		*/s_args.push_back(s_arg);
		/*2*/	s_arg = newSgpr("SzB", 2, 2);	/*s_load_dword(2, s_arg, s_argsAddr, 8 * 2 + 4 * 0);		*/s_args.push_back(s_arg);
		/*3*/	s_arg = newSgpr("D", 2, 2);		/*s_load_dword(2, s_arg, s_argsAddr, 8 * 3 + 4 * 0);		*/s_args.push_back(s_arg);
		/*4*/	s_arg = newSgpr("C", 2, 2);		/*s_load_dword(2, s_arg, s_argsAddr, 8 * 4 + 4 * 0);		*/s_args.push_back(s_arg);
		/*5*/	s_arg = newSgpr("A", 2, 2);		/*s_load_dword(2, s_arg, s_argsAddr, 8 * 5 + 4 * 0);		*/s_args.push_back(s_arg);
		/*6*/	s_arg = newSgpr("B", 2, 2);		/*s_load_dword(2, s_arg, s_argsAddr, 8 * 6 + 4 * 0);		*/s_args.push_back(s_arg);
		/*7*/	s_arg = newSgpr("alpha");		/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 0);		*/s_args.push_back(s_arg);
		/*8*/	s_arg = newSgpr("beta");		/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 1);		*/s_args.push_back(s_arg);
		/*9*/	s_arg = newSgpr("StrideD0");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 2);		*/s_args.push_back(s_arg);
		/*10*/	s_arg = newSgpr("StrideD1");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 3);		*/s_args.push_back(s_arg);
		/*11*/	s_arg = newSgpr("StrideC0");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 4);		*/s_args.push_back(s_arg);
		/*12*/	s_arg = newSgpr("StrideC1");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 5);		*/s_args.push_back(s_arg);
		/*13*/	s_arg = newSgpr("StrideA0");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 6);		*/s_args.push_back(s_arg);
		/*14*/	s_arg = newSgpr("StrideA1");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 7);		*/s_args.push_back(s_arg);
		/*15*/	s_arg = newSgpr("StrideB0");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 8);		*/s_args.push_back(s_arg);
		/*16*/	s_arg = newSgpr("StrideB1");	/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 9);		*/s_args.push_back(s_arg);
		/*17*/	s_arg = newSgpr("I");			/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 10);		*/s_args.push_back(s_arg);
		/*18*/	s_arg = newSgpr("J");			/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 11);		*/s_args.push_back(s_arg);
		/*19*/	s_arg = newSgpr("K");			/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 12);		*/s_args.push_back(s_arg);
		/*20*/	s_arg = newSgpr("sum");			/*s_load_dword(1, s_arg, s_argsAddr, 8 * 7 + 4 * 13);		*/s_args.push_back(s_arg);
												/*															*/
			s_arg = newSgpr("dbg", 2, 2);		/*/*s_load_dword(2, s_arg, s_argsAddr, 8 * 3 + 4 * 0);		*/s_args.push_back(s_arg);
			//s_wait_lgkmcnt(0);


		//	s_load_dword(1, s_args[k_arg_M], s_argsAddr, 8 * 7 + 4 * 10);
		//	s_load_dword(1, s_args[k_arg_N], s_argsAddr, 8 * 7 + 4 * 11);
			s_load_dword(1, s_args[k_arg_StrideD0], s_argsAddr, 8 * 7 + 4 * 2);
		//	s_load_dword(1, s_args[k_arg_StrideA0], s_argsAddr, 8 * 7 + 4 * 6);
		//	s_load_dword(1, s_args[k_arg_StrideB0], s_argsAddr, 8 * 7 + 4 * 8);
			s_load_dword(1, s_args[k_arg_K], s_argsAddr, 8 * 7 + 4 * 13);

			s_args_tmp = newSgpr("args_tmp", 4, 4);
			s_load_dword(4, s_args_tmp, s_argsAddr, 8 * 7 + 4 * 6);
			s_args_tmp = s_args_tmp ^ 1;

		}
		else
		{
			//f_load_kernel_args();
			s_args.clear();
			uint32_t bias = 0;
			T_Var s_arg;
			T_Var zhanwei1 = newSgpr("pA");
			T_Var zhanwei2 = newSgpr("pA", 2, 2);
			s_arg = newSgpr("pA", 2, 4);	s_args.push_back(s_arg);
			s_arg = newSgpr("pB", 2, 2);	s_args.push_back(s_arg);
			s_arg = newSgpr("pD", 2, 4);	s_args.push_back(s_arg);
			s_arg = newSgpr("pDbu", 2, 2);	s_args.push_back(s_arg);
			s_arg = newSgpr("M", 1, 4);		s_args.push_back(s_arg);
			s_arg = newSgpr("N");			s_args.push_back(s_arg);
			s_arg = newSgpr("K");			s_args.push_back(s_arg);
			s_arg = newSgpr("StA");			s_args.push_back(s_arg);
			s_arg = newSgpr("StB", 1, 2);	s_args.push_back(s_arg);
			s_arg = newSgpr("StD");			s_args.push_back(s_arg);
			delVar(zhanwei1);
			delVar(zhanwei2);

			// A/B
		//	s_load_dword(4, s_args[0] ^ 4, s_argsAddr, bias); 
			bias += 4 * 4;
			// D/Dbg
		//	s_load_dword(4, s_args[2] ^ 4, s_argsAddr, bias); 
			bias += 4 * 4;
			// M/N/K/StrideA
			s_load_dword(4, s_args[4] ^ 4, s_argsAddr, bias); bias += 4 * 4;
			// StrideB/StrideD
			s_load_dword(2, s_args[8] ^ 2, s_argsAddr, bias); 

			s_args[0] = s_args[0] ^ 1;
			s_args[1] = s_args[1] ^ 1;
			s_args[2] = s_args[2] ^ 1;
			s_args[4] = s_args[4] ^ 1;
			s_args[8] = s_args[8] ^ 1;
		}

		// =======================================================================
		// work item index  
		// ======================================================================= 	
		v_tmp1 = newVgpr("v_tmp1"); v_tmp2 = newVgpr("v_tmp2", 2, 2);
		s_tmp1 = newSgpr("s_tmp1"); s_tmp2 = newSgpr("s_tmp2", 2, 2);
		v_tid_in_wave = newVgpr("tid_in_wave");
		v_wvid_in_grp = newVgpr("wvid_in_grp");
		s_wvid_in_grp = newSgpr("wvid_in_grp");
		op3("v_and_b32", v_tid_in_wave, WAVE_SIZE - 1, v_tid_x);// tid_in_wave = tid % WAVE_SIZE
		op3("v_lshrrev_b32", v_wvid_in_grp, log2Int(WAVE_SIZE), v_tid_x);// wvid_in_grp = tid / WAVE_SIZE
		op2("v_readfirstlane_b32", s_wvid_in_grp, v_wvid_in_grp);

		// =======================================================================
		// gemm body
		// =======================================================================
		write_double_wave_program();

		// =======================================================================
		// free gpr 
		// =======================================================================
		//free_gprs();

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState checkKernelParameters()
	{
		//
		if (k_param.enTensileLayout == true)
		{
			//tensor2dSizeC = 0;
			//tensor2dSizeA = 1;
			//tensor2dSizeB = 2;
			//dataD = 3;
			//dataC = 4;
			//dataA = 5;
			//dataB = 6;
			//alpha = 7;
			//beta = 8;
			//strideD1J = 9;
			//strideD2K = 10;
			//strideC1J = 11;
			//strideC2K = 12;
			//strideA1I = 13;
			//strideA2K = 14;
			//strideB1J = 15;
			//strideB2K = 16;
			//sizeI = 17;
			//sizeJ = 18;
			//sizeK = 19;
			//sizeL = 20;
			//tensor2dSizeC = 21;
			//tensor2dSizeA = 22;
			//tensor2dSizeB = 23;
			//staggerUIter = 24;

			k_arg_pA = 5;
			k_arg_pB = 6;
			k_arg_pD = 3;
			k_arg_M = 17;
			k_arg_N = 18;
			k_arg_K = 20;
			k_arg_StrideA0 = 13;
			k_arg_StrideB0 = 15;
			k_arg_StrideD0 = 9;
			k_arg_pDbg = 21;
		}
		else
		{
			// [0]const float * A
			// [1]const float * B
			// [2]float * D
			// [3]float * dbg_buff
			// [4]const M
			// [5]const N
			// [6]const K
			// [7]const StrideA0 >= K
			// [8]const StrideB0 >= K
			// [9]const StrideD0 >= M
			uint32_t tmpcnt = 0;
			k_arg_pA = tmpcnt++;
			k_arg_pB = tmpcnt++;
			k_arg_pD = tmpcnt++;
			k_arg_pDbg = tmpcnt++;
			k_arg_M = tmpcnt++;
			k_arg_N = tmpcnt++;
			k_arg_K = tmpcnt++;
			k_arg_StrideA0 = tmpcnt++;
			k_arg_StrideB0 = tmpcnt++;
			k_arg_StrideD0 = tmpcnt++;
		}

		if (en_dirct_glb_to_lds)
		{
			if (k_param.dataType == E_DataType::Fp32)
			{
				glb_load_instr_dw_sz = 1;
				glb_store_instr_dw_sz = 4;
				lds_wr_instr_dw_sz = 1;
				lds_rd_instr_dw_sz = 1;
			}
			else if (k_param.dataType == E_DataType::Fp16)
			{
				glb_load_instr_dw_sz = 1;
				glb_store_instr_dw_sz = 2;
				lds_wr_instr_dw_sz = 1;
				lds_rd_instr_dw_sz = 2;
			}
			else if (k_param.dataType == E_DataType::Bf16)
			{
				glb_load_instr_dw_sz = 1;
				glb_store_instr_dw_sz = 2;
				lds_wr_instr_dw_sz = 1;
				lds_rd_instr_dw_sz = 1;
			}
			glb_load_instr_sz = glb_load_instr_dw_sz * GPR_SZ;
			glb_store_instr_sz = glb_store_instr_dw_sz * GPR_SZ;
			lds_wr_instr_sz = lds_wr_instr_dw_sz * GPR_SZ;
			lds_rd_instr_sz = lds_rd_instr_dw_sz * GPR_SZ;
		}
		else
		{
			glb_load_instr_dw_sz = 4;
			glb_store_instr_dw_sz = 4;
			lds_wr_instr_dw_sz = 4;
			lds_rd_instr_dw_sz = 4;
			glb_load_instr_sz = glb_load_instr_dw_sz * GPR_SZ;
			glb_store_instr_sz = glb_store_instr_dw_sz * GPR_SZ;
			lds_wr_instr_sz = lds_wr_instr_dw_sz * GPR_SZ;
			lds_rd_instr_sz = lds_rd_instr_dw_sz * GPR_SZ;
		}

		// workload
		math_wave_num_per_grp = wv_pttn0_per_grp * wv_pttn1_per_grp;

		elem_num0_per_grp = mfma_m * mfma_pttn0_per_wv * wv_pttn0_per_grp;
		elem_num1_per_grp = mfma_n * mfma_pttn1_per_wv * wv_pttn1_per_grp;
		mfma_blk0_per_grp = elem_num0_per_grp / mfma_m;
		mfma_blk1_per_grp = elem_num1_per_grp / mfma_n;
		mfma_blk_per_wv = mfma_blk0_per_grp * mfma_blk1_per_grp / math_wave_num_per_grp;
		mfma_k_times_per_wv = k_param.DepthU / mfma_k;

		// matrix a
		a_fetch_ele_num0_per_wv = k_param.DepthU;
		a_fetch_wave_shape0 = a_fetch_ele_num0_per_wv * elem_sz / GPR_SZ;
		a_fetch_wave_shape1 = WAVE_SIZE / a_fetch_wave_shape0;
		a_fetch_ele_num1_per_wv = a_fetch_wave_shape1;
		a_fetch_times = elem_num0_per_grp / math_wave_num_per_grp / a_fetch_ele_num1_per_wv;

		a_lds_sz_per_wv_per_time = WAVE_SIZE * GPR_SZ + lds_pad_byte;
		a_lds_sz_per_wv = a_lds_sz_per_wv_per_time * a_fetch_times;
		a_lds_sz_per_grp = a_lds_sz_per_wv * math_wave_num_per_grp;

		a_lds_read_step_1 = mfma_k * elem_sz;
		a_lds_read_step_2 = (mfma_m / a_fetch_ele_num1_per_wv) * a_lds_sz_per_wv_per_time;// 因为有padding，所以按照存储的行数进行计算

		// matrix b
		b_fetch_ele_num0_per_wv = k_param.DepthU;
		b_fetch_wave_shape0 = b_fetch_ele_num0_per_wv * elem_sz / GPR_SZ;
		b_fetch_wave_shape1 = WAVE_SIZE / b_fetch_wave_shape0;
		b_fetch_ele_num1_per_wv = b_fetch_wave_shape1;
		b_fetch_times = elem_num1_per_grp / math_wave_num_per_grp / b_fetch_ele_num1_per_wv;

		b_lds_sz_per_wv_per_time = WAVE_SIZE * GPR_SZ + lds_pad_byte;
		b_lds_sz_per_wv = b_lds_sz_per_wv_per_time * b_fetch_times;
		b_lds_sz_per_grp = b_lds_sz_per_wv * math_wave_num_per_grp;

		b_lds_read_step_1 = mfma_k * elem_sz;
		b_lds_read_step_2 = (mfma_n / b_fetch_ele_num1_per_wv) * b_lds_sz_per_wv_per_time;

		// mfma
		mfma_dgpr_per_mfma = mfma_m * mfma_n * mfma_b / WAVE_SIZE;
		mfma_agpr_per_mfma = mfma_m * mfma_k * mfma_b * elem_sz / GPR_SZ / WAVE_SIZE;
		mfma_bgpr_per_mfma = mfma_n * mfma_k * mfma_b * elem_sz / GPR_SZ / WAVE_SIZE;

		mfma_sz0_per_wv = mfma_pttn0_per_wv * mfma_m;
		mfma_sz1_per_wv = mfma_pttn1_per_wv * mfma_n;

		a_mfma_times = mfma_k_times_per_wv * mfma_pttn0_per_wv;
		b_mfma_times = mfma_k_times_per_wv * mfma_pttn1_per_wv;
		//		a_mfma_times = 1 * mfma_pttn0_per_wv;
		//		b_mfma_times = 1 * mfma_pttn1_per_wv;

				// interface
		if (IsaArch >= E_IsaArch::Gfx908)	en_mfma = true;
		else								en_mfma = false;
		if (k_param.enTensileLayout == true)
		{
			switch (k_param.dataType)
			{
			case E_DataType::Fp32:
				kernelName = "Cijk_Alik_Bljk_SB_MT";
				break;
			case E_DataType::Fp16:
				kernelName = "Cijk_Alik_Bljk_HBH_MT";
				break;
			case E_DataType::Bf16:
				kernelName = "Cijk_Alik_Bljk_BBH_MT";
				break;
			default:
				kernelName = "Cijk_Alik_Bljk_SB_MT";
				break;
			}

			kernelName += std::to_string(elem_num0_per_grp) + "x" + std::to_string(elem_num1_per_grp) + "x" + std::to_string(k_param.DepthU);
			kernelName += "_SE_K1";
		}
		else
		{
			kernelName += std::to_string(elem_num0_per_grp) + "x" + std::to_string(elem_num1_per_grp) + "x" + std::to_string(k_param.DepthU);
			kernelName += "_2wave";
		}

		group_sz = dim(math_wave_num_per_grp * WAVE_SIZE * 2, 1, 1);
		group_num = dim(k_param.M / elem_num0_per_grp, k_param.N / elem_num1_per_grp, 1);

		if ((k_param.M % elem_num0_per_grp != 0) || (k_param.N % elem_num1_per_grp != 0))
			return E_ReturnState::RTN_ERR;

		global_sz = group_sz * group_num;
		dispatch = T_Dispatch(global_sz, group_sz);

		return E_ReturnState::SUCCESS;
	}
	void LogKernelInnerParam()
	{
		LOG("kernel name = " + kernelName);
		LOG("mfma tile per wave  = [%d, %d].", mfma_pttn0_per_wv, mfma_pttn1_per_wv);
		LOG("wave tile per group = [%d, %d].", wv_pttn0_per_grp, wv_pttn1_per_grp);
		LOG("mfma tile per group = [%d, %d].", mfma_blk0_per_grp, mfma_blk1_per_grp);
		LOG("MT = [%d, %d].", elem_num0_per_grp, elem_num1_per_grp);
	}

private:
	void write_sigle_wave_program()
	{
	}
	void write_double_wave_program()
	{
		// =======================================================================
		// get hardware wave id
		// =======================================================================
		T_Var s_branch_id = newSgpr("s_branch_id");
		op3("s_lshr_b32", s_branch_id, s_wvid_in_grp, log2Int(math_wave_num_per_grp));// branch_id = wave_id / SIMD_PER_CU

		// ----------------------------------------------------
		// thread re-order
		if (math_wave_num_per_grp == 1)
		{
			op2("s_mov_b32", s_wvid_in_grp, 0);
			op2("v_mov_b32", v_wvid_in_grp, 0);
			op3("v_lshlrev_b32", v_tmp1, log2Int(WAVE_SIZE), v_wvid_in_grp);
			op3("v_add_u32", v_tid_x, v_tmp1, v_tid_in_wave);
		}
		if (math_wave_num_per_grp == 2)
		{
			op3("s_and_b32", s_tmp1, 1, s_wvid_in_grp);
			op2("s_mov_b32", s_wvid_in_grp, s_tmp1);
			op2("v_mov_b32", v_wvid_in_grp, s_tmp1);
			op3("v_lshlrev_b32", v_tmp1, log2Int(WAVE_SIZE), v_wvid_in_grp);
			op3("v_add_u32", v_tid_x, v_tmp1, v_tid_in_wave);
		}
		if (math_wave_num_per_grp >= SIMD_PER_CU)
		{
		//	op2("s_mov_b32", s_wvid_in_grp, s_hw_simd_id);
		//	op2("v_mov_b32", v_wvid_in_grp, s_hw_simd_id);
			op3("s_and_b32", s_tmp1, 3, s_wvid_in_grp);
			op2("s_mov_b32", s_wvid_in_grp, s_tmp1);
			op2("v_mov_b32", v_wvid_in_grp, s_tmp1);
			op3("v_lshlrev_b32", v_tmp1, log2Int(WAVE_SIZE), v_wvid_in_grp);
			op3("v_add_u32", v_tid_x, v_tmp1, v_tid_in_wave);
		}

		// ----------------------------------------------------
		// wave 0 jump to math segment
		T_Var l_math_wave = newLaber("MATH_WAVE");
		op2("s_cmp_eq_u32", s_branch_id, 0);// if(branch_id == 0) scc = 1;
		op1("s_cbranch_scc1", l_math_wave); // if(scc == 1) goto math_wave;
		delVar(s_branch_id); // 11240-11332 clk

		// ----------------------------------------------------
		write_fetch_wave_program();

		wrLaber(l_math_wave);
		write_math_wave_program();
	}
	void write_fetch_wave_program()
	{
		s_wait_lgkmcnt(0);
		if (k_param.enTensileLayout == true)
		{
			op2("s_mov_b32", s_args[k_arg_StrideA0], s_args_tmp + 0);
			op2("s_mov_b32", s_args[k_arg_StrideB0], s_args_tmp + 2);
			s_args_tmp = s_args_tmp ^ 4;
			delVar(s_args_tmp);
		}
		address_a_t0();
		address_b_n0();
		s_wait_lgkmcnt(0);

		if (k_param.lds_buffer_num == 2)
		{
			fetch_loop2();
		}
		if (k_param.lds_buffer_num == 3)
		{
			fetch_loop3();
		}
		op0("s_endpgm");
		free_fetch_gprs();
	}
	void write_math_wave_program()
	{
		acc_mfma_d = newAgpr("mfma_d", mfma_dgpr_per_mfma * mfma_blk_per_wv);

		// =======================================================================
		// init acc gpr
		// =======================================================================
		for (uint32_t i = 0; i < mfma_dgpr_per_mfma * mfma_blk_per_wv; i++)
			op2("v_accvgpr_write", acc_mfma_d + i, 0);
		
		s_wait_lgkmcnt(0);
		address_a_t1();
		address_b_n1();
		address_d_n(); 
		s_wait_lgkmcnt(0);

		if (k_param.lds_buffer_num == 2)
		{
			math_loop2();
		}
		if (k_param.lds_buffer_num == 3)
		{
			math_loop3();
		}

		free_math_gprs();
	}

	// =======================================================================
	// =======================================================================
	
	void address_a_t0()
	{
		// =======================================================================
		// A: global fetch A address 
		// =======================================================================	
		{
			// ---------------------------------------------------------------------
			// setup descrapter a
			s_a_dscp = newSgpr("dscr_a", 4, 4);
			if (k_param.enTensileLayout == true)
			{
				s_load_dword(2, s_a_dscp ^ 2, s_argsAddr ^ 2, 8 * 5 + 4 * 0);
			}
			else
			{
				s_load_dword(2, s_a_dscp ^ 2, s_argsAddr ^ 2, 0);
				//op2("s_mov_b32", s_a_dscp + 0, s_args[k_arg_pA] + 0);
				//op2("s_mov_b32", s_a_dscp + 1, s_args[k_arg_pA] + 1);
			}
			s_a_dscp = s_a_dscp ^ 1;
			op2h("s_mov_b32", s_a_dscp + 2, 0x80000000);
			op2h("s_mov_b32", s_a_dscp + 3, 0x00020000);
			s_a_dscp = s_a_dscp ^ 4;

			// ---------------------------------------------------------------------
			// group base row index
			T_Var s_a_grp_row_id_base = newSgpr("a_grp_row_base");
			op3("s_lshl_b32", s_a_grp_row_id_base, s_bid_x, log2Int(elem_num0_per_grp));// group_row_base = MT0 * group_id_x

			// wave base row index in group
			T_Var v_a_wv_row_id_in_grp = newVgpr("a_wv_row_in_grp");
			op3("v_lshlrev_b32", v_a_wv_row_id_in_grp, log2Int(a_fetch_ele_num1_per_wv * a_fetch_times), s_wvid_in_grp); // wave_row_id_in_group = wave_id * 每个wave分配的行数 = wave_id * (每个wave每次读取的行数 * 读取次数)

			// thread row & col in wave (element)
			T_Var v_a_trd_row_id_in_wv = newVgpr("a_trd_row_id_in_wv");
			T_Var v_a_trd_col_id_in_wv = newVgpr("a_trd_col_id_in_wv");
			op3("v_lshrrev_b32", v_a_trd_row_id_in_wv, log2Int(a_fetch_wave_shape0), v_tid_in_wave); // thread_row_id_in_wave = thread_in_wave / 每行thread数
			op3("v_and_b32", v_a_trd_col_id_in_wv, a_fetch_wave_shape0 - 1, v_tid_in_wave); // thread_col = thread_in_wave % 每行thread数

			// thread row & col address offset (byte)
			T_Var v_a_trd_row_id = newVgpr("a_trd_row_id");
			T_Var v_a_row_base_addr = newVgpr("s_a_row_base_addr");
			T_Var v_a_col_offset_addr = newVgpr("s_a_col_offset_addr");
			op4("v_add3_u32", v_a_trd_row_id, s_a_grp_row_id_base, v_a_wv_row_id_in_grp, v_a_trd_row_id_in_wv);// thread_row_id = group_row_base + wave_row_id_in_group + thread_row_id_in_wave
			op3("v_mul_lo_u32", v_tmp1, s_args[k_arg_StrideA0], v_a_trd_row_id);
			op3("v_lshlrev_b32", v_a_row_base_addr, log2Int(elem_sz), v_tmp1);
			op3("v_lshlrev_b32", v_a_col_offset_addr, log2Int(glb_load_instr_sz), v_a_trd_col_id_in_wv);

			// fetch global address step for different times (byte)
			T_Var s_a_addr_step = newSgpr("a_offset");
			op3("s_lshl_b32", s_a_addr_step, s_args[k_arg_StrideA0], log2Int(a_fetch_ele_num1_per_wv) + log2Int(elem_sz)); // fetch_step(element) = 每个wave每次读取的行数 * stride; BYTE(fetch_step)
			op3("s_sub_u32", s_a_addr_step, s_a_addr_step, a_lds_sz_per_wv_per_time);// correct address offset by lds offset (byte)

			// thread fetch address offset (byte)
			v_a_fetch_offset = newVgpr("a_trd_row_id_in_wv", a_fetch_times);
			op3("v_add_u32", v_a_fetch_offset, v_a_row_base_addr, v_a_col_offset_addr);
			for (uint32_t i = 0; i < a_fetch_times - 1; i++)
				op3("v_add_u32", v_a_fetch_offset + (i + 1), s_a_addr_step, v_a_fetch_offset + i);

			//f_debug_data(s_args[3], v_a_row_base_addr, true);
			delVar(s_a_grp_row_id_base);
			delVar(v_a_wv_row_id_in_grp);
			delVar(v_a_trd_row_id_in_wv);
			delVar(v_a_trd_col_id_in_wv);
			delVar(v_a_row_base_addr);
			delVar(v_a_col_offset_addr);
			delVar(v_a_trd_row_id);
			delVar(s_a_addr_step);
		}

		// =======================================================================
		// A: lds write address 
		// =======================================================================
		{
			ldsAllocByte(a_lds_sz_per_grp * k_param.lds_buffer_num); // *2 for ping-pang buffer		

			// ping pang buffer adderss (byte)
			s_a_lds_write = newSgpr("a_lds_write");
			s_a_lds_write_0 = newSgpr("a_lds_addr_ping");
			s_a_lds_write_1 = newSgpr("a_lds_addr_pang");
			s_a_lds_write_2 = newSgpr("a_lds_addr_pang");
			op2("s_mov_b32", s_tmp1, a_lds_sz_per_wv);
			op3("s_mul_i32", s_a_lds_write_0, s_wvid_in_grp, s_tmp1); // lds_ping_addr = wave_id * a_lds_size_per_wave
			op3("s_add_i32", s_a_lds_write_1, s_a_lds_write_0, a_lds_sz_per_grp); // lds_pang_addr = lds_ping_addr + lds_a_size_per_group
			if (k_param.lds_buffer_num == 2)
			{
				op3("s_xor_b32", s_a_lds_write_2, s_a_lds_write_0, s_a_lds_write_1);
			}
			else if (k_param.lds_buffer_num == 3)
			{
				op3("s_add_i32", s_a_lds_write_2, s_a_lds_write_1, a_lds_sz_per_grp);
			}
		}
	}
	void address_a_t1()
	{
		// =======================================================================
		// A: lds read address 
		// =======================================================================
		{
			// ---------------------------------------------------------------------
			// wave base row & col index in group
			T_Var v_a_wv_row_id_in_wvpttn = newVgpr("mfma_col_id");
			T_Var v_a_wv_row_id_in_grp = newVgpr("mfma_row_id");
			op3("v_and_b32", v_a_wv_row_id_in_wvpttn, wv_pttn0_per_grp - 1, s_wvid_in_grp);
			op3("v_lshlrev_b32", v_a_wv_row_id_in_grp, log2Int(mfma_sz0_per_wv), v_a_wv_row_id_in_wvpttn);

			// thread row & col in the first mfma in a wave (element)
			T_Var v_a_trd_row_id_in_mfma = newVgpr("row_id_in_mt");
			T_Var v_a_trd_col_id_in_mfma = newVgpr("col_id_in_mt");
			op3("v_and_b32", v_a_trd_row_id_in_mfma, v_tid_in_wave, mfma_m - 1); // row_id_in_mfma = tid_in_wave % mfma_m
			op3("v_lshrrev_b32", v_a_trd_col_id_in_mfma, log2Int(mfma_m), v_tid_in_wave);// col_id_in_mfma(element) = tid_in_wave / mfma_m * mfma_k

			// thread row & col address offset (byte)
			T_Var v_a_trd_row_id = newVgpr("a_trd_row_id");
			T_Var v_a_row_base_addr = newVgpr("s_a_row_base_addr");
			T_Var v_a_col_offset_addr = newVgpr("s_a_col_offset_addr");
			op3("v_add_u32", v_a_trd_row_id, v_a_wv_row_id_in_grp, v_a_trd_row_id_in_mfma);
			op3("v_lshlrev_b32", v_tmp1, log2Int(k_param.DepthU), v_a_trd_row_id); // row_id_in_mfma * depthU
			op3("v_lshlrev_b32", v_a_row_base_addr, log2Int(elem_sz), v_tmp1);
			op3("v_lshlrev_b32", v_a_col_offset_addr, log2Int(lds_rd_instr_sz), v_a_trd_col_id_in_mfma);

			// thread index offset (element)
		//	T_Var v_a_trd_row_id = newVgpr("row_id_in_mt");
		//	T_Var v_a_trd_idx = newVgpr("col_id_in_mt");
		//	op3("v_add_u32", v_a_trd_row_id, v_a_wv_row_id_in_grp, v_a_trd_row_id_in_mfma);
		//	op3("v_lshlrev_b32", v_tmp1, log2Int(k_param.DepthU), v_a_trd_row_id); // row_id_in_mfma * depthU
		//	op3("v_add_u32", v_a_trd_idx, v_tmp1, v_a_trd_col_id_in_mfma);// index_in_mt_lds(element) = row_id_in_mfma * depthU + col_id_in_mfma

			// lds pad offset (byte)
			T_Var v_a_lds_pad_offset = newVgpr("mfma_col_id");
			op3("v_lshrrev_b32", v_tmp1, log2Int(a_fetch_ele_num1_per_wv), v_a_trd_row_id);
			op3("v_mul_lo_u32", v_a_lds_pad_offset, lds_pad_byte, v_tmp1);

			// ---------------------------------------------------------------------
			v_a_lds_read = newVgpr("a_lds_offset");
			v_a_lds_read_0 = newVgpr("a_lds_offset");
			v_a_lds_read_1 = newVgpr("a_lds_offset");
			v_a_lds_read_2 = newVgpr("a_lds_offset");
			op3("v_add_u32", v_a_lds_read_0, v_a_row_base_addr, v_a_col_offset_addr);
			op3("v_add_u32", v_a_lds_read_0, v_a_lds_read_0, v_a_lds_pad_offset);
			op3("v_add_u32", v_a_lds_read_1, a_lds_sz_per_grp, v_a_lds_read_0);
			if (k_param.lds_buffer_num == 2)
			{
				op3("v_xor_b32", v_a_lds_read_2, v_a_lds_read_1, v_a_lds_read_0);
			}
			else if (k_param.lds_buffer_num == 3)
			{
				op3("v_add_u32", v_a_lds_read_2, a_lds_sz_per_grp, v_a_lds_read_1);
			}

			//f_debug_data(s_args[3], v_a_col_offset_addr, true);
			delVar(v_a_wv_row_id_in_wvpttn);
			delVar(v_a_wv_row_id_in_grp);
			delVar(v_a_trd_row_id_in_mfma);
			delVar(v_a_trd_col_id_in_mfma);
			delVar(v_a_trd_row_id);
			delVar(v_a_row_base_addr);
			delVar(v_a_col_offset_addr);
			delVar(v_a_lds_pad_offset);
		}
	}

	void address_b_n0()
	{
		// =======================================================================
		// B: global fetch A address
		// =======================================================================
		{
			// ---------------------------------------------------------------------
			// setup descrapter b
			s_b_dscp = newSgpr("dscr_b", 4, 4);
			if (k_param.enTensileLayout == true)
			{
				s_load_dword(2, s_b_dscp ^ 2, s_argsAddr ^ 2, 8 * 6 + 4 * 0);
			}
			else
			{
				s_load_dword(2, s_b_dscp ^ 2, s_argsAddr ^ 2, 4 * 2);
				//op2("s_mov_b32", s_b_dscp + 0, s_args[k_arg_pB] + 0);
				//op2("s_mov_b32", s_b_dscp + 1, s_args[k_arg_pB] + 1);
			}
			s_b_dscp = s_b_dscp ^ 1;
			op2h("s_mov_b32", s_b_dscp + 2, 0x80000000);
			op2h("s_mov_b32", s_b_dscp + 3, 0x00020000);
			s_b_dscp = s_b_dscp ^ 4;

			// ---------------------------------------------------------------------
			// group base row index
			T_Var s_b_grp_row_id_base = newSgpr("b_grp_row_base");
			op3("s_lshl_b32", s_b_grp_row_id_base, s_bid_y, log2Int(elem_num1_per_grp));

			// wave base row index in group
			T_Var v_b_wv_row_id_in_grp = newVgpr("b_wv_row_in_grp");
			op3("v_lshlrev_b32", v_b_wv_row_id_in_grp, log2Int(b_fetch_ele_num1_per_wv * b_fetch_times), s_wvid_in_grp);

			// thread row & col in wave (element)
			T_Var v_b_trd_row_id_in_wv = newVgpr("b_trd_row_id_in_wv");
			T_Var v_b_trd_col_id_in_wv = newVgpr("b_trd_col_id_in_wv");
			op3("v_lshrrev_b32", v_b_trd_row_id_in_wv, log2Int(b_fetch_wave_shape0), v_tid_in_wave);
			op3("v_and_b32", v_b_trd_col_id_in_wv, b_fetch_wave_shape0 - 1, v_tid_in_wave);

			// thread row & col address offset (byte)
			T_Var v_b_trd_row_id = newVgpr("b_trd_row_id");
			T_Var v_b_row_base_addr = newVgpr("s_b_row_base_addr");
			T_Var v_b_col_offset_addr = newVgpr("s_b_col_offset_addr");
			op4("v_add3_u32", v_b_trd_row_id, s_b_grp_row_id_base, v_b_wv_row_id_in_grp, v_b_trd_row_id_in_wv);
			op3("v_mul_lo_u32", v_tmp1, s_args[k_arg_StrideB0], v_b_trd_row_id);
			op3("v_lshlrev_b32", v_b_row_base_addr, log2Int(elem_sz), v_tmp1);
			op3("v_lshlrev_b32", v_b_col_offset_addr, log2Int(glb_load_instr_sz), v_b_trd_col_id_in_wv);

			// fetch global address step for different times (byte)
			T_Var s_b_addr_step = newSgpr("b_offset");
			op3("s_lshl_b32", s_b_addr_step, s_args[k_arg_StrideB0], log2Int(b_fetch_ele_num1_per_wv) + log2Int(elem_sz));
			op3("s_sub_u32", s_b_addr_step, s_b_addr_step, b_lds_sz_per_wv_per_time);// correct address offset by lds offset (byte)

			// thread fetch address offset (byte)
			v_b_fetch_offset = newVgpr("b_trd_row_id_in_wv", b_fetch_times);
			op3("v_add_u32", v_b_fetch_offset, v_b_row_base_addr, v_b_col_offset_addr);
			for (uint32_t i = 0; i < b_fetch_times - 1; i++)
				op3("v_add_u32", v_b_fetch_offset + (i + 1), s_b_addr_step, v_b_fetch_offset + i);

			delVar(s_b_grp_row_id_base);
			delVar(v_b_wv_row_id_in_grp);
			delVar(v_b_trd_row_id_in_wv);
			delVar(v_b_trd_col_id_in_wv);
			delVar(v_b_trd_row_id);
			delVar(v_b_row_base_addr);
			delVar(v_b_col_offset_addr);
			delVar(s_b_addr_step);
		}

		// =======================================================================
		// B: lds write address 
		// =======================================================================
		{
			ldsAllocByte(b_lds_sz_per_grp * k_param.lds_buffer_num);

			// ping pang buffer adderss (byte)
			s_b_lds_write = newSgpr("b_lds_addr_ping");
			s_b_lds_write_0 = newSgpr("b_lds_addr_ping");
			s_b_lds_write_1 = newSgpr("b_lds_addr_pang");
			s_b_lds_write_2 = newSgpr("b_lds_addr_pang");
			op2("s_mov_b32", s_tmp1, b_lds_sz_per_wv);
			op3("s_mul_i32", s_b_lds_write_0, s_wvid_in_grp, s_tmp1);
			op2("s_mov_b32", s_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
			op3("s_add_u32", s_b_lds_write_0, s_b_lds_write_0, s_tmp1); // lds_ping_addr += a_lds_size * 2; *2 for ping-pang buffer
			op3("s_add_i32", s_b_lds_write_1, s_b_lds_write_0, b_lds_sz_per_grp);
			if (k_param.lds_buffer_num == 2)
			{
				op3("s_xor_b32", s_b_lds_write_2, s_b_lds_write_0, s_b_lds_write_1);
			}
			else if (k_param.lds_buffer_num == 3)
			{
				op3("s_add_i32", s_b_lds_write_2, s_b_lds_write_1, b_lds_sz_per_grp);
			}
		}
	}
	void address_b_n1()
	{
		// =======================================================================
		// B: lds read address 
		// =======================================================================
		{
			// ---------------------------------------------------------------------
			// wave base row & col index in group
			T_Var v_b_wv_col_id_in_wvpttn = newVgpr("mfma_col_id");
			T_Var v_b_wv_row_id_in_grp = newVgpr("mfma_row_id");
			op3("v_lshrrev_b32", v_b_wv_col_id_in_wvpttn, log2Int(wv_pttn0_per_grp), s_wvid_in_grp);
			op3("v_lshlrev_b32", v_b_wv_row_id_in_grp, log2Int(mfma_sz1_per_wv), v_b_wv_col_id_in_wvpttn);

			// thread row & col in the first mfma in a wave (element)
			T_Var v_b_trd_row_id_in_mfma = newVgpr("row_id_in_mt");
			T_Var v_b_trd_col_id_in_mfma = newVgpr("col_id_in_mt");
			op3("v_and_b32", v_b_trd_row_id_in_mfma, v_tid_in_wave, mfma_m - 1); // row_id_in_mfma = tid_in_wave % mfma_m
			op3("v_lshrrev_b32", v_b_trd_col_id_in_mfma, log2Int(mfma_m), v_tid_in_wave);// col_id_in_mfma(element) = tid_in_wave / mfma_m * mfma_k

			// thread index offset (element)
			T_Var v_b_trd_row_id = newVgpr("row_id_in_mt");
			T_Var v_b_row_base_addr = newVgpr("s_b_row_base_addr");
			T_Var v_b_col_offset_addr = newVgpr("s_b_col_offset_addr");
			op3("v_add_u32", v_b_trd_row_id, v_b_wv_row_id_in_grp, v_b_trd_row_id_in_mfma);
			op3("v_lshlrev_b32", v_tmp1, log2Int(k_param.DepthU), v_b_trd_row_id);
			op3("v_lshlrev_b32", v_b_row_base_addr, log2Int(elem_sz), v_tmp1);
			op3("v_lshlrev_b32", v_b_col_offset_addr, log2Int(lds_rd_instr_sz), v_b_trd_col_id_in_mfma);

			// lds pad offset (byte)
			T_Var v_b_lds_pad_offset = newVgpr("mfma_col_id");
			op3("v_lshrrev_b32", v_tmp1, log2Int(b_fetch_ele_num1_per_wv), v_b_trd_row_id);
			op3("v_mul_lo_u32", v_b_lds_pad_offset, lds_pad_byte, v_tmp1);

			// ---------------------------------------------------------------------
			v_b_lds_read = newVgpr("a_lds_offset");
			v_b_lds_read_0 = newVgpr("a_lds_offset");
			v_b_lds_read_1 = newVgpr("a_lds_offset");
			v_b_lds_read_2 = newVgpr("a_lds_offset");
			op3("v_add_u32", v_b_lds_read_0, v_b_row_base_addr, v_b_col_offset_addr);
			op2h("v_mov_b32", v_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num); // *2 is for ping pang buffer A			
			op3("v_add_u32", v_b_lds_read_0, v_b_lds_read_0, v_tmp1);
			op3("v_add_u32", v_b_lds_read_0, v_b_lds_read_0, v_b_lds_pad_offset);
			op3("v_add_u32", v_b_lds_read_1, b_lds_sz_per_grp, v_b_lds_read_0);
			if (k_param.lds_buffer_num == 2)
			{
				op3("v_xor_b32", v_b_lds_read_2, v_b_lds_read_1, v_b_lds_read_0);
			}
			else if (k_param.lds_buffer_num == 3)
			{
				op3("v_add_u32", v_b_lds_read_2, b_lds_sz_per_grp, v_b_lds_read_1);
			}

			delVar(v_b_wv_col_id_in_wvpttn);
			delVar(v_b_wv_row_id_in_grp);
			delVar(v_b_trd_row_id_in_mfma);
			delVar(v_b_trd_col_id_in_mfma);
			delVar(v_b_trd_row_id);
			delVar(v_b_row_base_addr);
			delVar(v_b_col_offset_addr);
			delVar(v_b_lds_pad_offset);
		}
	}

	void address_d_n()
	{
		// =======================================================================
		// calculate D address 
		// =======================================================================
		{
			// -----------------------------------------------------------------------
			// setup descrapter d
			s_d_dscp = newSgpr("dscr_d", 4, 4);
			if (k_param.enTensileLayout == true)
			{
				s_load_dword(2, s_d_dscp ^ 2, s_argsAddr ^ 2, 8 * 3 + 4 * 0);
			}
			else
			{
				s_load_dword(2, s_d_dscp ^ 2, s_argsAddr ^ 2, 4 * 4);
				//op2("s_mov_b32", s_d_dscp + 0, s_args[k_arg_pD] + 0);
				//op2("s_mov_b32", s_d_dscp + 1, s_args[k_arg_pD] + 1);
			}
			s_d_dscp = s_d_dscp ^ 1;
			op2h("s_mov_b32", s_d_dscp + 2, 0x80000000);
			op2h("s_mov_b32", s_d_dscp + 3, 0x00020000);
			s_d_dscp = s_d_dscp ^ 4;

			// -----------------------------------------------------------------------
			// group base row & col index (element)
			T_Var s_d_grp_row_id_base = newSgpr("d_grp_row_base");
			T_Var s_d_grp_col_id_base = newSgpr("d_col_grp_base");
			op3("s_lshl_b32", s_d_grp_col_id_base, s_bid_x, log2Int(elem_num0_per_grp * elem_sz / GPR_SZ));//DWORD group_col_base = MT0*group_id_x (element); 横向每个group的起始列号
			op3("s_lshl_b32", s_d_grp_row_id_base, s_bid_y, log2Int(elem_num1_per_grp));// group_row_base = MT1*group_id_y ;

			// wave base row index in group (DWORD)
			T_Var v_d_wv_row_id_in_grp = newVgpr("d_wv_row_in_grp");
			T_Var v_d_wv_col_id_in_grp = newVgpr("d_wv_col_in_grp");
			op3("s_lshr_b32", s_tmp1, s_wvid_in_grp, log2Int(wv_pttn0_per_grp)); // wave_pttn_row_id_in_group = wave_id / wave_pattan0; wave 在 wave pattan of group 的第几行
			op3("v_lshlrev_b32", v_d_wv_row_id_in_grp, log2Int(mfma_sz1_per_wv), s_tmp1);// wave_row_id_in_group = mfma_size1_per_wave * wave_pttn_row_id_in_group
			op3("s_and_b32", s_tmp1, s_wvid_in_grp, wv_pttn0_per_grp - 1);// // wave_pttn_col_id_in_group = wave_id % wave_pattan0; wave 在 wave pattan of group 的第几列
			//op3("v_lshlrev_b32", v_d_wv_col_id_in_grp, log2Int(mfma_sz0_per_wv), s_tmp1);// wave_col_id_in_group = mfma_size0_per_wave * wave_pttn_col_id_in_group
			op3("v_lshlrev_b32", v_d_wv_col_id_in_grp, log2Int(mfma_sz0_per_wv * elem_sz / GPR_SZ), s_tmp1); // DWORD

			// thread row & col index in mfma (DWORD) 
			T_Var v_d_trd_row_id_in_mfma = newVgpr("d_trd_row_id_in_mfma");
			T_Var v_d_trd_lane_id_in_mfma = newVgpr("d_trd_lane_id_in_mfma");
			T_Var v_d_trd_col_id_in_mfma = newVgpr("d_trd_col_id_in_mfma");
			op3("v_and_b32", v_d_trd_row_id_in_mfma, mfma_m - 1, v_tid_in_wave); // thread_row_id_in_mfma = v_tid_in_wave % mfma_m; thread在mfma结果的纵向哪一行
			op3("v_lshrrev_b32", v_d_trd_lane_id_in_mfma, log2Int(mfma_m), v_tid_in_wave); // thread_lane_id_in_mfma = v_tid_in_wave / mfma_m; thread在mfma结果的横向哪一lane,(每lane是4列)
			//op3("v_lshlrev_b32", v_d_trd_col_id_in_mfma, log2Int(mfma_t), v_d_trd_lane_id_in_mfma); // thread_col_id_in_mfma = mfma_tt * thread_lane_id_in_mfma; thread在mfma结果的横向哪一列(element)
			op3("v_lshlrev_b32", v_d_trd_col_id_in_mfma, log2Int(mfma_t * elem_sz / GPR_SZ), v_d_trd_lane_id_in_mfma); // 每个lane = mfma_t 元素

			// thread index offset (element)
			T_Var v_d_trd_row_idx = newVgpr("d_trd_row_idx");
			T_Var v_d_trd_col_idx = newVgpr("d_trd_col_idx");
			op4("v_add3_u32", v_d_trd_row_idx, s_d_grp_row_id_base, v_d_wv_row_id_in_grp, v_d_trd_row_id_in_mfma);// thread_row_id = group_row_base + wave_row_id_in_group + thread_row_id_in_mfma
			op4("v_add3_u32", v_d_trd_col_idx, s_d_grp_col_id_base, v_d_wv_col_id_in_grp, v_d_trd_col_id_in_mfma);// thread_col_id = group_col_base + wave_col_id_in_group + thread_col_id_in_mfma

			// thread row & col address offset (byte)
			T_Var v_d_row_base_addr = newVgpr("s_b_row_base_addr");
			T_Var v_d_col_offset_addr = newVgpr("s_b_col_offset_addr");
			op3("v_mul_lo_u32", v_tmp1, s_args[k_arg_StrideD0], v_d_trd_row_idx);
			op3("v_lshlrev_b32", v_d_row_base_addr, log2Int(elem_sz), v_tmp1);
			op3("v_lshlrev_b32", v_d_col_offset_addr, log2Int(GPR_SZ), v_d_trd_col_idx);

			// global store step for different mfma_tt/mfma/mfma_pattan (byte)
			uint32_t mfma_lan_per_tt = WAVE_SIZE / mfma_m;
			mfma_dgpr_tt_sz = mfma_t * elem_sz / GPR_SZ;
			d_glb_step1 = mfma_t * elem_sz * mfma_lan_per_tt; // 同一笔mfma指令里每mfma_tt个vgpr的步进(Byte)
			if (k_param.dataType == E_DataType::Fp16)	d_glb_step1 = 16;
			if (k_param.dataType == E_DataType::Bf16)	d_glb_step1 = 16;
			d_glb_step2 = mfma_m * elem_sz; // MT0方向(不切换mfma_pattan行时),同thread同R0的步进(Byte),需要累加到立即数偏移
			T_Var v_d_glb_step3 = newVgpr("d_glb_step3");// MT1方向(切换mfma_pattan行时),同thread同R0的步进(Byte),需要累加到v_addr_offset上
			op3("v_lshlrev_b32", v_tmp1, log2Int(mfma_n), s_args[k_arg_StrideD0]);// d_glb_step3 = mfma_n * stride
			op3("v_lshlrev_b32", v_d_glb_step3, log2Int(elem_sz), v_tmp1);// d_glb_step3 = BYTE(d_glb_step3)

			// thread fetch address offset (byte)
			v_d_store_offset = newVgpr("d_store_offset", mfma_pttn1_per_wv);
			op3("v_add_u32", v_d_store_offset, v_d_row_base_addr, v_d_col_offset_addr);
			for (uint32_t i = 0; i < mfma_pttn1_per_wv - 1; i++)
				op3("v_add_u32", v_d_store_offset + (i + 1), v_d_glb_step3, v_d_store_offset + i);

			//f_debug_data(s_args[3], v_d_col_offset_addr, true);
			delVar(s_d_grp_row_id_base);
			delVar(s_d_grp_col_id_base);
			delVar(v_d_wv_row_id_in_grp);
			delVar(v_d_wv_col_id_in_grp);
			delVar(v_d_trd_row_id_in_mfma);
			delVar(v_d_trd_lane_id_in_mfma);
			delVar(v_d_trd_col_id_in_mfma);
			delVar(v_d_trd_row_idx);
			delVar(v_d_trd_col_idx);
			delVar(v_d_row_base_addr);
			delVar(v_d_col_offset_addr);
			delVar(v_d_glb_step3);
		}
	}

	void switch_lds_write()
	{
		if (k_param.lds_buffer_num == 2)
		{
			op3("s_xor_b32", s_a_lds_write, s_a_lds_write_2, s_a_lds_write);
			op3("s_xor_b32", s_b_lds_write, s_b_lds_write_2, s_b_lds_write);
			op3("s_xor_b32", s_a_lds_write_1, s_a_lds_write_2, s_a_lds_write);
			op3("s_xor_b32", s_b_lds_write_1, s_b_lds_write_2, s_b_lds_write);
		}
		else if (k_param.lds_buffer_num == 3)
		{
			static uint32_t lab_cnt;
			T_Var l_sem1 = newLaber(std::string("LDS_WR_SWITCH_1") + std::to_string(lab_cnt));
			T_Var l_sem2 = newLaber(std::string("LDS_WR_SWITCH_2") + std::to_string(lab_cnt));
			T_Var l_sem_e = newLaber(std::string("LDS_WR_SWITCH_3") + std::to_string(lab_cnt));

			op2("s_cmp_eq_i32", lds_buffer_cnt, 1);
			op1("s_cbranch_scc1", l_sem1);
			op2("s_cmp_eq_i32", lds_buffer_cnt, 2);
			op1("s_cbranch_scc1", l_sem2);

			op2("s_mov_b32", lds_buffer_cnt, 1);
			op2("s_mov_b32", s_a_lds_write, s_a_lds_write_1);
			op2("s_mov_b32", s_b_lds_write, s_b_lds_write_1);
			op1("s_branch", l_sem_e);

			wrLaber(l_sem1);
			op2("s_cmp_eq_i32", lds_buffer_cnt, 1);
			op2("s_cmov_b32", lds_buffer_cnt, 2);
			op2("s_cmov_b32", s_a_lds_write, s_a_lds_write_2);
			op2("s_cmov_b32", s_b_lds_write, s_b_lds_write_2);
			op1("s_cbranch_scc1", l_sem_e);

			wrLaber(l_sem2);
			op2("s_mov_b32", lds_buffer_cnt, 0);
			op2("s_mov_b32", s_a_lds_write, s_a_lds_write_0);
			op2("s_mov_b32", s_b_lds_write, s_b_lds_write_0);

			wrLaber(l_sem_e);
			lab_cnt++;
		}
	}
	void move_to_next_glb_fetch()
	{
		for (uint32_t i = 0; i < a_fetch_times; i++)
			op3("v_add_u32", v_a_fetch_offset + i, k_param.DepthU * elem_sz, v_a_fetch_offset + i);
		for (uint32_t i = 0; i < b_fetch_times; i++)
			op3("v_add_u32", v_b_fetch_offset + i, k_param.DepthU * elem_sz, v_b_fetch_offset + i);
	}
	void fetch_glb_to_lds_enter_loop()
	{
		op2("s_mov_b32", "m0", s_a_lds_write);
		for (uint32_t i = 0; i < a_fetch_times; i++)
			buffer_load_dword(1, v_tmp1, v_a_fetch_offset + i, s_a_dscp, 0, false, true, true, a_lds_sz_per_wv_per_time * i);

		op2("s_mov_b32", "m0", s_b_lds_write);
		for (uint32_t i = 0; i < b_fetch_times; i++)
			buffer_load_dword(1, v_tmp1, v_b_fetch_offset + i, s_b_dscp, 0, false, true, true, b_lds_sz_per_wv_per_time * i);

		move_to_next_glb_fetch();
		switch_lds_write();
	}
	void fetch_glb_to_lds_loop()
	{
		op2("s_mov_b32", "m0", s_a_lds_write);
		for (uint32_t i = 0; i < a_fetch_times; i++)
			buffer_load_dword(1, v_tmp1, v_a_fetch_offset + i, s_a_dscp, 0, false, true, true, a_lds_sz_per_wv_per_time * i);

		op2("s_mov_b32", "m0", s_b_lds_write);
		for (uint32_t i = 0; i < b_fetch_times; i++)
			buffer_load_dword(1, v_tmp1, v_b_fetch_offset + i, s_b_dscp, 0, false, true, true, b_lds_sz_per_wv_per_time * i);

		move_to_next_glb_fetch();
		switch_lds_write();
		if (k_param.lds_buffer_num == 3)
			s_wait_vmcnt(fetch_glb_waitcnt);
		op0("s_barrier");
	}

	void switch_lds_read()
	{
		if (k_param.lds_buffer_num == 2)
		{
			op3("v_xor_b32", v_a_lds_read, v_a_lds_read_2, v_a_lds_read);
			op3("v_xor_b32", v_b_lds_read, v_b_lds_read_2, v_b_lds_read);
			op3("v_xor_b32", v_a_lds_read_1, v_a_lds_read_2, v_a_lds_read);
			op3("v_xor_b32", v_b_lds_read_1, v_b_lds_read_2, v_b_lds_read);
		}
		else if (k_param.lds_buffer_num == 3)
		{
			static uint32_t lab_cnt;
			T_Var l_lds12 = newLaber(std::string("LDS_RD_SWITCH_1") + std::to_string(lab_cnt));
			T_Var l_lds_e = newLaber(std::string("LDS_RD_SWITCH_2") + std::to_string(lab_cnt));
			op3("v_cmp_ne_u32", "vcc", v_a_lds_read, v_a_lds_read_0);
			op1("s_cbranch_vccnz", l_lds12);
			op2("v_mov_b32", v_a_lds_read, v_a_lds_read_1);
			op2("v_mov_b32", v_b_lds_read, v_b_lds_read_1);
			op1("s_branch", l_lds_e);
			wrLaber(l_lds12);
			op3("v_cmp_eq_u32", "vcc", v_a_lds_read, v_a_lds_read_1);
			op4("v_cndmask_b32", v_a_lds_read, v_a_lds_read_0, v_a_lds_read_2, "vcc");
			op4("v_cndmask_b32", v_b_lds_read, v_b_lds_read_0, v_b_lds_read_2, "vcc");
			wrLaber(l_lds_e);
			lab_cnt++;
		}
	}
	void read_lds_to_mfma_ping(uint32_t k)
	{
		for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			ds_read_dword(lds_rd_instr_dw_sz, v_mfma_a_ping + (lds_rd_instr_dw_sz * m), v_a_lds_read, a_lds_read_step_1 * k + a_lds_read_step_2 * m);

		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
			ds_read_dword(lds_rd_instr_dw_sz, v_mfma_b_ping + (lds_rd_instr_dw_sz * n), v_b_lds_read, b_lds_read_step_1 * k + b_lds_read_step_2 * n);
	}
	void read_lds_to_mfma_pang(uint32_t k)
	{
		for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			ds_read_dword(lds_rd_instr_dw_sz, v_mfma_a_pang + (lds_rd_instr_dw_sz * m), v_a_lds_read, a_lds_read_step_1 * k + a_lds_read_step_2 * m);

		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
			ds_read_dword(lds_rd_instr_dw_sz, v_mfma_b_pang + (lds_rd_instr_dw_sz * n), v_b_lds_read, b_lds_read_step_1 * k + b_lds_read_step_2 * n);
	}
	void mfma_mfma_ping()
	{
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t a_idx_offset = mfma_agpr_per_mfma * m;
				uint32_t b_idx_offset = mfma_bgpr_per_mfma * n;
				uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n * mfma_pttn0_per_wv + m);

				op4(mfma_inst,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
					(v_mfma_a_ping + a_idx_offset) ^ mfma_agpr_per_mfma,
					(v_mfma_b_ping + b_idx_offset) ^ mfma_bgpr_per_mfma,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);
			}
		}
	}
	void mfma_mfma_pang()
	{
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t a_idx_offset = mfma_agpr_per_mfma * m;
				uint32_t b_idx_offset = mfma_bgpr_per_mfma * n;
				uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n * mfma_pttn0_per_wv + m);

				op4(mfma_inst,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
					(v_mfma_a_pang + a_idx_offset) ^ mfma_agpr_per_mfma,
					(v_mfma_b_pang + b_idx_offset) ^ mfma_bgpr_per_mfma,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);
			}
		}
	}
	void mfma_mfma_ping_with_lds_switch()
	{
		uint32_t num = mfma_pttn1_per_wv * mfma_pttn0_per_wv;
		uint32_t cnt = 0;
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t a_idx_offset = mfma_agpr_per_mfma * m;
				uint32_t b_idx_offset = mfma_bgpr_per_mfma * n;
				uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n * mfma_pttn0_per_wv + m);

				op4(mfma_inst,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
					(v_mfma_a_ping + a_idx_offset) ^ mfma_agpr_per_mfma,
					(v_mfma_b_ping + b_idx_offset) ^ mfma_bgpr_per_mfma,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);

				cnt++;
				if ((cnt == num - 1) || (num == 1))
				{
					switch_lds_read();
				}
			}
		}
	}
	void mfma_mfma_pang_with_read_lds()
	{
		uint32_t cnt = 0;
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t a_idx_offset = mfma_agpr_per_mfma * m;
				uint32_t b_idx_offset = mfma_bgpr_per_mfma * n;
				uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n * mfma_pttn0_per_wv + m);

				op4(mfma_inst,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
					(v_mfma_a_pang + a_idx_offset) ^ mfma_agpr_per_mfma,
					(v_mfma_b_pang + b_idx_offset) ^ mfma_bgpr_per_mfma,
					(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);

				cnt++;
				if (cnt == 1)
				{
					read_lds_to_mfma_ping(0);
				}
			}
		}
	}
	void mfma_mfma_pang_with_store()
	{
		v_rslt_d = newVgpr("rslt_d", mfma_dgpr_per_mfma, 2);
		T_Var v_bf16_msk = newVgpr("bf16_msk");
		op2h("v_mov_b32", v_bf16_msk, 0xffff0000);

		uint32_t loop_num = mfma_pttn1_per_wv * mfma_pttn0_per_wv;
		uint32_t loop_cnt = 0;
		uint32_t m0 = 0, n0 = 0, m1 = 0, n1 = 0;

		loop_cnt = 0;
		{
			uint32_t a_idx_offset = mfma_agpr_per_mfma * m0;
			uint32_t b_idx_offset = mfma_bgpr_per_mfma * n0;
			uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n0 * mfma_pttn0_per_wv + m0);
			uint32_t mfma_d_idx = (m1 + n1 * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

			op4(mfma_inst,
				(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
				(v_mfma_a_pang + a_idx_offset) ^ mfma_agpr_per_mfma,
				(v_mfma_b_pang + b_idx_offset) ^ mfma_bgpr_per_mfma,
				(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);

			m0++;
			if (m0 == mfma_pttn0_per_wv)
			{
				m0 = 0;
				n0++;
			}
		}

		for (loop_cnt = 1; loop_cnt < loop_num; loop_cnt++)
		{
			uint32_t a_idx_offset = mfma_agpr_per_mfma * m0;
			uint32_t b_idx_offset = mfma_bgpr_per_mfma * n0;
			uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n0 * mfma_pttn0_per_wv + m0);
			uint32_t mfma_d_idx = (m1 + n1 * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

			op4(mfma_inst,
				(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
				(v_mfma_a_pang + a_idx_offset) ^ mfma_agpr_per_mfma,
				(v_mfma_b_pang + b_idx_offset) ^ mfma_bgpr_per_mfma,
				(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);

			for (uint32_t i = 0; i < mfma_dgpr_per_mfma; i++)
			{
				op2("v_accvgpr_read", v_rslt_d + i, (acc_mfma_d + (mfma_d_idx + i)) ^ 1);

				if (k_param.dataType == E_DataType::Fp32) // fp32
				{
					op1("s_nop", 4);
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(glb_store_instr_dw_sz,
							v_rslt_d + 4 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				else if (k_param.dataType == E_DataType::Fp16) // fp16
				{
					op2("v_cvt_f16_f32", v_rslt_d + i, v_rslt_d + i);
					if ((i + 1) % 2 == 0)
					{
						op3("v_pack_b32_f16", v_rslt_d + (i / 2), v_rslt_d + (i - 1), v_rslt_d + i);
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(glb_store_instr_dw_sz,
							v_rslt_d + 2 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				else if (k_param.dataType == E_DataType::Bf16) // bf16
				{
					if ((i + 1) % 2 == 0)
					{
						op3("v_lshrrev_b32", v_rslt_d + (i - 1), 16, v_rslt_d + (i - 1));
						op4("v_and_or_b32", v_rslt_d + (i / 2), v_rslt_d + i, v_bf16_msk, v_rslt_d + (i - 1));
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(glb_store_instr_dw_sz,
							v_rslt_d + 2 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
			}

			m0++;
			if (m0 == mfma_pttn0_per_wv)
			{
				m0 = 0;
				n0++;
			}
			m1++;
			if (m1 == mfma_pttn0_per_wv)
			{
				m1 = 0;
				n1++;
			}
		}

		loop_cnt = loop_num - 1;
		{
			uint32_t a_idx_offset = mfma_agpr_per_mfma * m0;
			uint32_t b_idx_offset = mfma_bgpr_per_mfma * n0;
			uint32_t acc_idx_offset = mfma_dgpr_per_mfma * (n0 * mfma_pttn0_per_wv + m0);
			uint32_t mfma_d_idx = (m1 + n1 * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

			if (loop_num == 1)
			{
				op1("s_nop", 32);
			}

			for (uint32_t i = 0; i < mfma_dgpr_per_mfma; i++)
			{
				op2("v_accvgpr_read", v_rslt_d + i, (acc_mfma_d + (mfma_d_idx + i)) ^ 1);

				if (k_param.dataType == E_DataType::Fp32) // fp32
				{
					op1("s_nop", 4);
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(glb_store_instr_dw_sz,
							v_rslt_d + 4 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				else if (k_param.dataType == E_DataType::Fp16) // fp16
				{
					op2("v_cvt_f16_f32", v_rslt_d + i, v_rslt_d + i);
					if ((i + 1) % 2 == 0)
					{
						op3("v_pack_b32_f16", v_rslt_d + (i / 2), v_rslt_d + (i - 1), v_rslt_d + i);
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(glb_store_instr_dw_sz,
							v_rslt_d + 2 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				else if (k_param.dataType == E_DataType::Bf16) // bf16
				{
					if ((i + 1) % 2 == 0)
					{
						op3("v_lshrrev_b32", v_rslt_d + (i - 1), 16, v_rslt_d + (i - 1));
						op4("v_and_or_b32", v_rslt_d + (i / 2), v_rslt_d + i, v_bf16_msk, v_rslt_d + (i - 1));
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(glb_store_instr_dw_sz,
							v_rslt_d + 2 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
			}
		}
	}

	void math_lds_loop()
	{
		// read ping (do it in pre-loop)
		//read_lds_to_mfma_ping(0);
		// read pang
		read_lds_to_mfma_pang(1);

		// loop
		for (uint32_t k = 0; k < k_param.DepthU / mfma_k / 2 - 1; k++)
		{
			// wait ping n' math ping
			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_ping();

			// read ping
			read_lds_to_mfma_ping(2 * (k + 1) + 0);

			// wait pang n' math pang
			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_pang();

			// read pang
			read_lds_to_mfma_pang(2 * (k + 1) + 1);
		}

		// wait ping n' math ping
		s_wait_lgkmcnt(read_lds_waitcnt);
		mfma_mfma_ping_with_lds_switch();

		// wait pang n' math pang
		if (k_param.lds_buffer_num == 3)
			s_wait_lgkmcnt(0);
		op0("s_barrier");
		mfma_mfma_pang_with_read_lds();
	}
	void math_lds_exit_loop()
	{
		// read ping (do it in pre-loop)
		//read_lds_to_mfma_ping(0);
		// read pang
		read_lds_to_mfma_pang(1);

		// loop
		for (uint32_t k = 0; k < k_param.DepthU / mfma_k / 2 - 1; k++)
		{
			// wait ping n' math ping
			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_ping();

			// read ping
			read_lds_to_mfma_ping(2 * (k + 1) + 0);

			// wait pang n' math pang
			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_pang();

			// read pang
			read_lds_to_mfma_pang(2 * (k + 1) + 1);
		}

		// wait ping n' math ping
		s_wait_lgkmcnt(read_lds_waitcnt);
		mfma_mfma_ping();

		// wait pang n' math pang
		s_wait_lgkmcnt(0);
		mfma_mfma_pang_with_store();
		
		//mfma_mfma_pang();
		//store_result();
	}
		
	void fetch_loop2()
	{
		fetch_glb_waitcnt = 0;

		lds_buffer_cnt = newSgpr("lds_buffer_cnt");
		op2("s_mov_b32", lds_buffer_cnt, 0);
		op2("s_mov_b32", s_a_lds_write, s_a_lds_write_0);
		op2("s_mov_b32", s_b_lds_write, s_b_lds_write_0);

		// =======================================================================
		// enter loop
		// =======================================================================

		// =======================================================================
		// loop
		// =======================================================================
		T_Var l_end_fetch_loop = newLaber("END_FETCH_LOOP");
		op3("s_lshr_b32", s_tmp1, s_args[k_arg_K], log2Int(k_param.DepthU));

		T_Var s_fetch_loop_cnt = f_s_loop(s_tmp1, "FETCH_LOOP");
		{
			fetch_glb_to_lds_loop();
		}
		f_e_loop(s_fetch_loop_cnt, "FETCH_LOOP");

		// =======================================================================
		// exit loop
		// =======================================================================
		//op0("s_barrier");
	}
	void math_loop2()
	{
		if (read_lds_waitcnt < 0)
			read_lds_waitcnt = mfma_pttn0_per_wv + mfma_pttn1_per_wv;

		v_mfma_a_ping = newVgpr("mfma_a", mfma_agpr_per_mfma * a_mfma_times);
		v_mfma_b_ping = newVgpr("mfma_b", mfma_bgpr_per_mfma * b_mfma_times);
		v_mfma_a_pang = newVgpr("mfma_a", mfma_agpr_per_mfma * a_mfma_times);
		v_mfma_b_pang = newVgpr("mfma_b", mfma_bgpr_per_mfma * b_mfma_times);
		/*acc_mfma_d = newAgpr("mfma_d", mfma_dgpr_per_mfma * mfma_blk_per_wv);

		// =======================================================================
		// init acc gpr
		// =======================================================================
		for (uint32_t i = 0; i < mfma_dgpr_per_mfma * mfma_blk_per_wv; i++)
			op2("v_accvgpr_write", acc_mfma_d + i, 0);*/

		op1("s_setprio", 1);
		op2("v_mov_b32", v_a_lds_read, v_a_lds_read_0);
		op2("v_mov_b32", v_b_lds_read, v_b_lds_read_0);

		// =======================================================================
		// enter loop
		// =======================================================================
		op0("s_barrier");// fetch ready
		read_lds_to_mfma_ping(0);

		// =======================================================================
		// loop
		// =======================================================================
		T_Var l_end_math_loop = newLaber("END_MATH_LOOP");
		op3("s_lshr_b32", s_tmp1, s_args[k_arg_K], log2Int(k_param.DepthU)); // *2 for ping pang unroll		
		op2("s_cmp_le_u32", s_tmp1, 1); // if(loop_cnt <= 1) scc = 1;
		op1("s_cbranch_scc1", l_end_math_loop);// if(scc == 1) no loop;
		op3("s_sub_u32", s_tmp1, s_tmp1, 1);// sub enter and exit loop

		T_Var s_gemm_loop_cnt = f_s_loop(s_tmp1, "MATH_LOOP");
		{
			math_lds_loop();
		}
		f_e_loop(s_gemm_loop_cnt, "MATH_LOOP");
		wrLaber(l_end_math_loop);

		// =======================================================================
		// exit loop
		// =======================================================================
	//	op0("s_barrier");// fetch ready
		math_lds_exit_loop();
	}

	void fetch_loop3()
	{
		if (fetch_glb_waitcnt < 0)
			fetch_glb_waitcnt = (a_fetch_times + b_fetch_times) * 1;
		if (fetch_glb_waitcnt > 63)
			fetch_glb_waitcnt = 63;

		lds_buffer_cnt = newSgpr("lds_buffer_cnt");
		op2("s_mov_b32", lds_buffer_cnt, 0);
		op2("s_mov_b32", s_a_lds_write, s_a_lds_write_0);
		op2("s_mov_b32", s_b_lds_write, s_b_lds_write_0);

		//	fetch_glb_to_lds_test();
		//	fetch_glb_to_lds_test();
		//	return;
			// =======================================================================
			// enter loop
			// =======================================================================
		fetch_glb_to_lds_enter_loop();	// fetch 0

		// =======================================================================
		// loop
		// =======================================================================
		T_Var l_end_fetch_loop = newLaber("END_FETCH_LOOP");
		op3("s_lshr_b32", s_tmp1, s_args[k_arg_K], log2Int(k_param.DepthU));
		op2("s_cmp_le_u32", s_tmp1, 1); // if(loop_cnt <= 1) scc = 1;
		op1("s_cbranch_scc1", l_end_fetch_loop);// if(scc == 1) no loop;
		op3("s_sub_u32", s_tmp1, s_tmp1, 1);// sub enter and exit loop

		T_Var s_fetch_loop_cnt = f_s_loop(s_tmp1, "FETCH_LOOP");
		{
			fetch_glb_to_lds_loop();
		}
		f_e_loop(s_fetch_loop_cnt, "FETCH_LOOP");
		wrLaber(l_end_fetch_loop);

		// =======================================================================
		// exit loop
		// =======================================================================
		s_wait_vmcnt(0); op0("s_barrier"); // fetch last ready
	}
	void math_loop3()
	{
		if (read_lds_waitcnt < 0)
			read_lds_waitcnt = mfma_pttn0_per_wv + mfma_pttn1_per_wv;

		v_mfma_a_ping = newVgpr("mfma_a", mfma_agpr_per_mfma * a_mfma_times);
		v_mfma_b_ping = newVgpr("mfma_b", mfma_bgpr_per_mfma * b_mfma_times);
		v_mfma_a_pang = newVgpr("mfma_a", mfma_agpr_per_mfma * a_mfma_times);
		v_mfma_b_pang = newVgpr("mfma_b", mfma_bgpr_per_mfma * b_mfma_times);
		acc_mfma_d = newAgpr("mfma_d", mfma_dgpr_per_mfma * mfma_blk_per_wv);

		// =======================================================================
		// init acc gpr
		// =======================================================================
		for (uint32_t i = 0; i < mfma_dgpr_per_mfma * mfma_blk_per_wv; i++)
			op2("v_accvgpr_write", acc_mfma_d + i, 0);

		op1("s_setprio", 1);
		op2("v_mov_b32", v_a_lds_read, v_a_lds_read_0);
		op2("v_mov_b32", v_b_lds_read, v_b_lds_read_0);

		//	math_lds_test();
		//	math_lds_test();
		//	return;
			// =======================================================================
			// enter loop
			// =======================================================================
		op0("s_barrier"); // fetch 0 ready
		read_lds_to_mfma_ping(0);

		// =======================================================================
		// loop
		// =======================================================================
		T_Var l_end_math_loop = newLaber("END_MATH_LOOP");
		op3("s_lshr_b32", s_tmp1, s_args[k_arg_K], log2Int(k_param.DepthU)); // *2 for ping pang unroll		
		op2("s_cmp_le_u32", s_tmp1, 1); // if(loop_cnt <= 1) scc = 1;
		op1("s_cbranch_scc1", l_end_math_loop);// if(scc == 1) no loop;
		op3("s_sub_u32", s_tmp1, s_tmp1, 1);// sub enter and exit loop

		T_Var s_gemm_loop_cnt = f_s_loop(s_tmp1, "MATH_LOOP");
		{
			math_lds_loop();
		}
		f_e_loop(s_gemm_loop_cnt, "MATH_LOOP");
		wrLaber(l_end_math_loop);

		// =======================================================================
		// exit loop
		// =======================================================================
		math_lds_exit_loop();
	}

	void store_result()
	{
		delVar(v_mfma_a_ping);
		delVar(v_mfma_b_ping);
		delVar(v_mfma_a_pang);
		delVar(v_mfma_b_pang);
		v_rslt_d = newVgpr("rslt_d", mfma_dgpr_per_mfma, 2);
		T_Var v_bf16_msk = newVgpr("bf16_msk");
		op2h("v_mov_b32", v_bf16_msk, 0xffff0000);

		op1("s_nop", 32);
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t mfma_d_idx = (m + n * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

				for (uint32_t i = 0; i < mfma_dgpr_per_mfma; i++)
				{
					op2("v_accvgpr_read", v_rslt_d + i, (acc_mfma_d + (mfma_d_idx + i)) ^ 1);

					if (k_param.dataType == E_DataType::Fp32) // fp32
					{
						op1("s_nop", 4);
						if ((i + 1) % 4 == 0)
						{
							uint32_t instr_offset = m * d_glb_step2 + d_glb_step1 * (i / 4);

							buffer_store_dword(glb_store_instr_dw_sz,
								v_rslt_d + 4 * (i / 4),
								v_d_store_offset + n,
								s_d_dscp, 0, false, true,
								instr_offset);
						}
					}
					else if (k_param.dataType == E_DataType::Fp16) // fp16
					{
						op2("v_cvt_f16_f32", v_rslt_d + i, v_rslt_d + i);
						if ((i + 1) % 2 == 0)
						{
							op3("v_pack_b32_f16", v_rslt_d + (i / 2), v_rslt_d + (i - 1), v_rslt_d + i);
						}
						if ((i + 1) % 4 == 0)
						{
							uint32_t instr_offset = m * d_glb_step2 + d_glb_step1 * (i / 4);

							buffer_store_dword(glb_store_instr_dw_sz,
								v_rslt_d + 2 * (i / 4),
								v_d_store_offset + n,
								s_d_dscp, 0, false, true,
								instr_offset);
						}
					}
					else if (k_param.dataType == E_DataType::Bf16) // bf16
					{
						if ((i + 1) % 2 == 0)
						{
							op3("v_lshrrev_b32", v_rslt_d + (i - 1), 16, v_rslt_d + (i - 1));
							op4("v_and_or_b32", v_rslt_d + (i / 2), v_rslt_d + i, v_bf16_msk, v_rslt_d + (i - 1));
						}
						if ((i + 1) % 4 == 0)
						{
							uint32_t instr_offset = m * d_glb_step2 + d_glb_step1 * (i / 4);

							buffer_store_dword(glb_store_instr_dw_sz,
								v_rslt_d + 2 * (i / 4),
								v_d_store_offset + n,
								s_d_dscp, 0, false, true,
								instr_offset);
						}
					}
				}
			}
		}
	}

	// =======================================================================
	// =======================================================================

	void free_fetch_gprs()
	{
		delVar(lds_buffer_cnt);

		delVar(s_a_dscp);
		delVar(v_a_fetch_offset);
		delVar(s_a_lds_write);
		delVar(s_a_lds_write_0);
		delVar(s_a_lds_write_1);
		delVar(s_a_lds_write_2);

		delVar(s_b_dscp);
		delVar(v_b_fetch_offset);
		delVar(s_b_lds_write);
		delVar(s_b_lds_write_0);
		delVar(s_b_lds_write_1);
		delVar(s_b_lds_write_2);
	}
	void free_math_gprs()
	{
		delVar(v_a_lds_read);
		delVar(v_a_lds_read_0);
		delVar(v_a_lds_read_1);
		delVar(v_a_lds_read_2);

		delVar(v_b_lds_read);
		delVar(v_b_lds_read_0);
		delVar(v_b_lds_read_1);
		delVar(v_b_lds_read_2);

		delVar(acc_mfma_d);

		delVar(v_rslt_d);
		delVar(s_d_dscp);
		delVar(v_d_store_offset);
	}
	void free_gprs()
	{
		delVar(v_tmp1); delVar(v_tmp2);
		delVar(s_tmp1);	delVar(s_tmp2);

		delVar(v_tid_in_wave);
		delVar(v_wvid_in_grp);
		delVar(s_wvid_in_grp);
	}
};

