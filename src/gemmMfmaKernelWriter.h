#pragma once

#include "../inc/KernelWriter.h"

using namespace feifei;

#define BF16_SZ	(2)
#define FP16_SZ	(2)
#define FP32_SZ	(4)

typedef struct T_GemmMfmaKernelParamType
{
	E_DataType DataType;
	bool enTensileLayout;

	uint32_t M, N, K, Batch;
	uint32_t loop_unroll;
	uint32_t mfma_mn;
	uint32_t mfma_pttn_per_wave[2];
	uint32_t wave_pttn_per_group[2];
	uint32_t lds_buffer_num;
}T_GemmMfmaKernelParam;

class GemmMfmaKernelWriter :public KernelWriter
{
public:
	GemmMfmaKernelWriter(T_GemmMfmaKernelParam kernelParam) :KernelWriter()
	{
		k_param = kernelParam;

		mfma_m = mfma_n = k_param.mfma_mn;
		if (k_param.DataType == E_DataType::Fp32)
		{
			elem_sz = FP32_SZ;
			c_elem_sz = FP32_SZ;
			if (k_param.mfma_mn == 16)	mfma_k = 4;
			if (k_param.mfma_mn == 32)	mfma_k = 2;
			if (k_param.mfma_mn == 16)	mfma_inst = "v_mfma_f32_16x16x4f32";
			if (k_param.mfma_mn == 32)	mfma_inst = "v_mfma_f32_32x32x2f32";
			kernelName = "sgemm_col_TN_MT";
		}
		if (k_param.DataType == E_DataType::Fp16)
		{
			elem_sz = FP16_SZ;
			c_elem_sz = FP16_SZ;
			if (k_param.mfma_mn == 16)	mfma_k = 16;
			if (k_param.mfma_mn == 32)	mfma_k = 8;
			if (k_param.mfma_mn == 16)	mfma_inst = "v_mfma_f32_16x16x16f16";
			if (k_param.mfma_mn == 32)	mfma_inst = "v_mfma_f32_32x32x8f16";
			kernelName = "hgemm_col_TN_MT";
		}
		if (k_param.DataType == E_DataType::Bf16)
		{
			elem_sz = BF16_SZ;
			c_elem_sz = FP32_SZ;
			if (k_param.mfma_mn == 16)	mfma_k = 8;
			if (k_param.mfma_mn == 32)	mfma_k = 4;
			if (k_param.mfma_mn == 16)	mfma_inst = "v_mfma_f32_16x16x8bf16";
			if (k_param.mfma_mn == 32)	mfma_inst = "v_mfma_f32_32x32x4bf16";
			kernelName = "bfgemm_col_TN_MT";
		}

		mfma_pttn0_per_wv = k_param.mfma_pttn_per_wave[0];
		mfma_pttn1_per_wv = k_param.mfma_pttn_per_wave[1];
		wv_pttn0_per_grp = k_param.wave_pttn_per_group[0];
		wv_pttn1_per_grp = k_param.wave_pttn_per_group[1];
	}

protected:
	T_GemmMfmaKernelParam k_param;

#pragma  region Variable
	uint32_t elem_sz;
	uint32_t c_elem_sz;

	// -----------------------------------------------------------------------
	uint32_t argidx_A, argbias_A;
	uint32_t argidx_B, argbias_B;
	uint32_t argidx_C, argbias_C;
	uint32_t argidx_D, argbias_D;
	uint32_t argidx_StrA0, argbias_StrA0;
	uint32_t argidx_StrB0, argbias_StrB0;
	uint32_t argidx_StrC0, argbias_StrC0;
	uint32_t argidx_StrD0, argbias_StrD0;
	uint32_t argidx_BtStrA, argbias_BtStrA;
	uint32_t argidx_BtStrB, argbias_BtStrB;
	uint32_t argidx_BtStrC, argbias_BtStrC;
	uint32_t argidx_BtStrD, argbias_BtStrD;
	uint32_t argidx_Alpha, argbias_Alpha;
	uint32_t argidx_Beta, argbias_Beta;
	uint32_t argidx_M, argbias_M;
	uint32_t argidx_N, argbias_N;
	uint32_t argidx_K, argbias_K;
	uint32_t argidx_Batch, argbias_Batch;

	// -----------------------------------------------------------------------
	uint32_t mfma_pttn0_per_wv;
	uint32_t mfma_pttn1_per_wv;
	uint32_t wv_pttn0_per_grp;
	uint32_t wv_pttn1_per_grp;
	uint32_t math_wave_num_per_grp;

	// -----------------------------------------------------------------------
	T_Var v_tmp1, v_tmp2;
	T_Var s_tmp1, s_tmp2;
	T_Var v_tid_in_wave;
	T_Var s_bid_in_glb;
	T_Var v_wvid_in_grp, s_wvid_in_grp;
	T_Var v_wvid_in_glb, s_wvid_in_glb;
	uint32_t total_grp_num;
	uint32_t total_wave_num;

	// -----------------------------------------------------------------------
	bool en_direct_glb_to_lds = true;

	int32_t fetch_glb_waitcnt = -1;
	int32_t write_lds_waitcnt = -1;
	int32_t read_lds_waitcnt = -1;

	// -----------------------------------------------------------------------
	uint32_t lds_wr_instr_dw_sz;
	uint32_t lds_wr_instr_sz;
	uint32_t lds_rd_instr_dw_sz;
	uint32_t lds_rd_instr_sz;

	uint32_t lds_pad_dw;
	uint32_t lds_pad_byte;
	uint32_t lds_nopad_dw_len;
	uint32_t lds_nopad_row_num;
	uint32_t lds_wr_nopad_trd_num;

	T_Var s_lds_write_cnt, s_lds_write_cnt_bck;
	T_Var v_lds_write_cnt, v_lds_write_cnt_bck;
	T_Var v_lds_read_cnt, v_lds_read_cnt_bck;

	// -----------------------------------------------------------------------
	T_Var s_a_dscp;
	T_Var v_a_fetch_offset;
	T_Var v_glb_load_addr_a, v_glb_load_a;
	T_Var s_a_lds_write, s_a_lds_write_0, s_a_lds_write_1, s_a_lds_write_2;
	T_Var v_a_lds_write, v_a_lds_write_0, v_a_lds_write_1, v_a_lds_write_2;
	T_Var v_a_lds_read, v_a_lds_read_0, v_a_lds_read_1, v_a_lds_read_2;
	uint32_t a_fetch_instr_dw_sz;
	uint32_t a_fetch_instr_sz;
	uint32_t a_fetch_ele_num0_per_wv;
	uint32_t a_fetch_ele_num1_per_wv;
	uint32_t a_fetch_wave_shape0;
	uint32_t a_fetch_wave_shape1;
	uint32_t a_fetch_times;

	uint32_t a_lds_sz_per_wv_per_time;
	uint32_t a_lds_sz_per_wv;
	uint32_t a_lds_sz_per_grp;
	uint32_t a_lds_write_step;
	uint32_t a_lds_read_step_1;
	uint32_t a_lds_read_step_2;

	// -----------------------------------------------------------------------
	T_Var s_b_dscp;
	T_Var v_b_fetch_offset;
	T_Var v_glb_load_addr_b, v_glb_load_b;
	T_Var s_b_lds_write, s_b_lds_write_0, s_b_lds_write_1, s_b_lds_write_2;
	T_Var v_b_lds_write, v_b_lds_write_0, v_b_lds_write_1, v_b_lds_write_2;
	T_Var v_b_lds_read, v_b_lds_read_0, v_b_lds_read_1, v_b_lds_read_2;
	uint32_t b_fetch_instr_dw_sz;
	uint32_t b_fetch_instr_sz;
	uint32_t b_fetch_ele_num0_per_wv;
	uint32_t b_fetch_ele_num1_per_wv;
	uint32_t b_fetch_wave_shape0;
	uint32_t b_fetch_wave_shape1;
	uint32_t b_fetch_times;

	uint32_t b_lds_sz_per_wv_per_time;
	uint32_t b_lds_sz_per_wv;
	uint32_t b_lds_sz_per_grp;
	uint32_t b_lds_write_step;
	uint32_t b_lds_read_step_1;
	uint32_t b_lds_read_step_2;

	// -----------------------------------------------------------------------
	T_Var v_alpha, v_beta;

	T_Var s_c_dscp;
	T_Var v_glb_load_c;
	T_Var v_c_fetch_offset;
	uint32_t c_fetch_instr_dw_sz;
	uint32_t c_fetch_instr_sz;
	uint32_t c_glb_step1;
	uint32_t c_glb_step2;

	// -----------------------------------------------------------------------
	T_Var s_d_dscp;
	T_Var v_rslt_d;
	T_Var v_d_store_offset;
	uint32_t d_store_instr_dw_sz;
	uint32_t d_store_instr_sz;
	uint32_t d_glb_step1;
	uint32_t d_glb_step2;

	// -----------------------------------------------------------------------
	std::string mfma_inst;
	uint32_t mfma_m, mfma_n;
	uint32_t mfma_k, mfma_b = 1;
	uint32_t mfma_lane_shape_n;
	uint32_t mfma_lane_shape_m = 4;
	uint32_t mfma_lane_num_per_tile;
	uint32_t mfma_tile_shape_n;
	uint32_t mfma_tile_shape_m;
	uint32_t mfma_tile_num_per_mfma;
	uint32_t mfma_dgpr_per_mfma;
	uint32_t mfma_agpr_per_mfma;
	uint32_t mfma_bgpr_per_mfma;
	T_Var v_mfma_a_ping, v_mfma_a_pang;
	T_Var v_mfma_b_ping, v_mfma_b_pang;
	T_Var acc_mfma_d;
	uint32_t mfma_blk0_per_grp;
	uint32_t mfma_blk1_per_grp;
	uint32_t mfma_k_times_per_wv;
	uint32_t mfma_blk_per_wv;
	uint32_t mfma_sz0_per_wv;
	uint32_t mfma_sz1_per_wv;
	uint32_t elem_num0_per_grp;
	uint32_t elem_num1_per_grp;
	uint32_t a_mfma_times;
	uint32_t b_mfma_times;
#pragma endregion

	E_ReturnState writeProgramDetail()
	{
		s_args.clear();
		uint32_t bias = 0;
		T_Var s_arg;
		T_Var zhanwei = newSgpr("zhanwei");
		s_arg = newSgpr("pD", 2, 2);	s_args.push_back(s_arg);
		s_arg = newSgpr("pC", 2, 2);	s_args.push_back(s_arg);
		s_arg = newSgpr("pA", 2, 2);	s_args.push_back(s_arg);
		s_arg = newSgpr("pB", 2, 2);	s_args.push_back(s_arg);
		s_arg = newSgpr("Alpha", 1, 2);	s_args.push_back(s_arg);
		s_arg = newSgpr("Beta");		s_args.push_back(s_arg);
		s_arg = newSgpr("StD", 1, 4);	s_args.push_back(s_arg);
		s_arg = newSgpr("BtStD");		s_args.push_back(s_arg);
		s_arg = newSgpr("StC");			s_args.push_back(s_arg);
		s_arg = newSgpr("BtStC");		s_args.push_back(s_arg);
		s_arg = newSgpr("StA", 1, 4);	s_args.push_back(s_arg);
		s_arg = newSgpr("BtStA");		s_args.push_back(s_arg);
		s_arg = newSgpr("StB");			s_args.push_back(s_arg);
		s_arg = newSgpr("BtStB");		s_args.push_back(s_arg);
		delVar(zhanwei);

		// =======================================================================
		v_tmp1 = newVgpr("v_tmp1"); v_tmp2 = newVgpr("v_tmp2", 2, 2);
		s_tmp1 = newSgpr("s_tmp1"); s_tmp2 = newSgpr("s_tmp2", 2, 2);
		v_tid_in_wave = newVgpr("tid_in_wave");
		v_wvid_in_grp = newVgpr("wvid_in_grp");
		s_wvid_in_grp = newSgpr("wvid_in_grp");
		op3("v_and_b32", v_tid_in_wave, WAVE_SIZE - 1, v_tid_x);
		op3("v_lshrrev_b32", v_wvid_in_grp, log2Int(WAVE_SIZE), v_tid_x);
		op2("v_readfirstlane_b32", s_wvid_in_grp, v_wvid_in_grp);
		total_grp_num = group_num.x * group_num.y;
		total_wave_num = total_grp_num * wv_pttn0_per_grp * wv_pttn1_per_grp;
		s_bid_in_glb = newSgpr("group_id_in_global");
		op3("s_mul_i32", s_tmp1, group_num.x, s_bid_y);
		op3("s_add_u32", s_bid_in_glb, s_tmp1, s_bid_x);
		s_wvid_in_glb = newSgpr("wave_id_in_global");
		op3("s_mul_i32", s_tmp1, wv_pttn0_per_grp * wv_pttn1_per_grp, s_bid_in_glb);
		op3("s_add_u32", s_wvid_in_glb, s_tmp1, s_wvid_in_grp);

		// ----------------------------------------------------
		T_Var s_branch_id = newSgpr("s_branch_id");
		op3("s_lshr_b32", s_branch_id, s_wvid_in_grp, log2Int(math_wave_num_per_grp));

		// ----------------------------------------------------
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
		if (math_wave_num_per_grp == 4)
		{
			op3("s_and_b32", s_tmp1, 3, s_wvid_in_grp);
			op2("s_mov_b32", s_wvid_in_grp, s_tmp1);
			op2("v_mov_b32", v_wvid_in_grp, s_tmp1);
			op3("v_lshlrev_b32", v_tmp1, log2Int(WAVE_SIZE), v_wvid_in_grp);
			op3("v_add_u32", v_tid_x, v_tmp1, v_tid_in_wave);
		}
		if (math_wave_num_per_grp == 8)
		{
			op3("s_and_b32", s_tmp1, 7, s_wvid_in_grp);
			op2("s_mov_b32", s_wvid_in_grp, s_tmp1);
			op2("v_mov_b32", v_wvid_in_grp, s_tmp1);
			op3("v_lshlrev_b32", v_tmp1, log2Int(WAVE_SIZE), v_wvid_in_grp);
			op3("v_add_u32", v_tid_x, v_tmp1, v_tid_in_wave);
		}

		// ----------------------------------------------------
		T_Var l_math_wave = newLaber("MATH_WAVE");
		op2("s_cmp_eq_u32", s_branch_id, 0);
		op1("s_cbranch_scc1", l_math_wave);
		delVar(s_branch_id);

		// ----------------------------------------------------
		fetch_wave();

		wrLaber(l_math_wave);
		math_wave();

		free_gprs();

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState checkKernelParameters()
	{
		uint32_t idxbase = 0;
		uint32_t biasbase = (k_param.enTensileLayout == true) ? 8 * 3 : 8 * 0;
		argidx_D = idxbase++;		argbias_D = biasbase + 8 * 0;
		argidx_C = idxbase++;		argbias_C = biasbase + 8 * 1;
		argidx_A = idxbase++;		argbias_A = biasbase + 8 * 2;
		argidx_B = idxbase++;		argbias_B = biasbase + 8 * 3;
		argidx_Alpha = idxbase++;	argbias_Alpha = biasbase + 8 * 4 + 4 * 0;
		argidx_Beta = idxbase++;	argbias_Beta = biasbase + 8 * 4 + 4 * 1;
		argidx_StrD0 = idxbase++;	argbias_StrD0 = biasbase + 8 * 4 + 4 * 2;
		argidx_BtStrD = idxbase++;	argbias_BtStrD = biasbase + 8 * 4 + 4 * 3;
		argidx_StrC0 = idxbase++;	argbias_StrC0 = biasbase + 8 * 4 + 4 * 4;
		argidx_BtStrC = idxbase++;	argbias_BtStrC = biasbase + 8 * 4 + 4 * 5;
		argidx_StrA0 = idxbase++;	argbias_StrA0 = biasbase + 8 * 4 + 4 * 6;
		argidx_BtStrA = idxbase++;	argbias_BtStrA = biasbase + 8 * 4 + 4 * 7;
		argidx_StrB0 = idxbase++;	argbias_StrB0 = biasbase + 8 * 4 + 4 * 8;
		argidx_BtStrB = idxbase++;	argbias_BtStrB = biasbase + 8 * 4 + 4 * 9;
		
		// ---------------------------------------------------------------------
		mfma_dgpr_per_mfma = mfma_m * mfma_n * mfma_b / WAVE_SIZE;
		mfma_agpr_per_mfma = mfma_m * mfma_k * mfma_b * elem_sz / GPR_SZ / WAVE_SIZE;
		mfma_bgpr_per_mfma = mfma_n * mfma_k * mfma_b * elem_sz / GPR_SZ / WAVE_SIZE;

		mfma_lane_shape_n = mfma_n;
		mfma_lane_shape_m = 4;
		mfma_lane_num_per_tile = WAVE_SIZE / mfma_lane_shape_n;
		mfma_tile_shape_n = mfma_lane_shape_n;
		mfma_tile_shape_m = mfma_lane_shape_m * mfma_lane_num_per_tile;
		mfma_tile_num_per_mfma = mfma_dgpr_per_mfma / mfma_lane_shape_m;

		mfma_sz0_per_wv = mfma_pttn0_per_wv * mfma_m;
		mfma_sz1_per_wv = mfma_pttn1_per_wv * mfma_n;

		mfma_k_times_per_wv = k_param.loop_unroll / mfma_k;
		a_mfma_times = mfma_k_times_per_wv * mfma_pttn0_per_wv;
		b_mfma_times = mfma_k_times_per_wv * mfma_pttn1_per_wv;

		// ---------------------------------------------------------------------
		bool status = true;
		status &= addr_at_0();
		status &= addr_bn_0();
		status &= addr_cn_0();
		status &= addr_dn_0();
		if (status == false)
		{
			WARN("wrong kernel parameters.");
			return E_ReturnState::RTN_ERR;
		}

		mfma_blk_per_wv = mfma_blk0_per_grp * mfma_blk1_per_grp / math_wave_num_per_grp;
		
		// ---------------------------------------------------------------------
		if (k_param.enTensileLayout == true)
		{
			if (k_param.DataType == E_DataType::Fp32)	kernelName = "Cijk_Alik_Bljk_SB_MT";
			if (k_param.DataType == E_DataType::Fp16)	kernelName = "Cijk_Alik_Bljk_HBH_MT";
			if (k_param.DataType == E_DataType::Bf16)	kernelName = "Cijk_Alik_Bljk_BBH_MT";

			kernelName += std::to_string(elem_num0_per_grp) + "x" + std::to_string(elem_num1_per_grp) + "x" + std::to_string(k_param.loop_unroll);
			kernelName += "_SE_K1";
		}
		else
		{
			kernelName += std::to_string(elem_num0_per_grp) + "x" + std::to_string(elem_num1_per_grp) + "x" + std::to_string(k_param.loop_unroll);
			kernelName += "_2wave";
		}

		group_sz = dim(math_wave_num_per_grp * WAVE_SIZE * 2, 1, 1);
		group_num = dim(k_param.M / elem_num0_per_grp, k_param.N / elem_num1_per_grp, k_param.Batch);

		if ((k_param.M % elem_num0_per_grp != 0) || (k_param.N % elem_num1_per_grp != 0))
			return E_ReturnState::RTN_ERR;
		if ((k_param.loop_unroll / mfma_k / 2.0) < 1)
			return E_ReturnState::RTN_ERR;
		if ((math_wave_num_per_grp > 4) && (k_param.lds_buffer_num > 3))
			return E_ReturnState::RTN_ERR;

		global_sz = group_sz * group_num;
		dispatch = T_Dispatch(global_sz, group_sz);

		return E_ReturnState::SUCCESS;
	}

private:
	void fetch_wave()
	{
		s_load_dword(2, s_args[argidx_A], s_argsAddr, argbias_A);
		s_load_dword(2, s_args[argidx_B], s_argsAddr, argbias_B);
		s_load_dword(4, s_args[argidx_StrA0] ^ 4, s_argsAddr, argbias_StrA0);
	
		s_args[argidx_A] = s_args[argidx_A] ^ 1;
		s_args[argidx_B] = s_args[argidx_B] ^ 1;
		s_args[argidx_StrA0] = s_args[argidx_StrA0] ^ 1;
		s_args[argidx_StrB0] = s_args[argidx_StrB0] ^ 1;
		s_args[argidx_BtStrA] = s_args[argidx_BtStrA] ^ 1;
		s_args[argidx_BtStrB] = s_args[argidx_BtStrB] ^ 1;

		// ---------------------------------------------------------------------
		addr_at_2();
		addr_bn_2();
		s_wait_lgkmcnt(0);
		addr_at_1();
		addr_bn_1();
		
		fetch_loop();

		op0("s_endpgm");
		free_fetch_gprs();
	}
	void math_wave()
	{
		acc_mfma_d = newAgpr("mfma_d", mfma_dgpr_per_mfma * mfma_blk_per_wv);
		for (uint32_t i = 0; i < mfma_dgpr_per_mfma * mfma_blk_per_wv; i++)
			op2("v_accvgpr_write", acc_mfma_d + i, 0);

		s_load_dword(2, s_args[argidx_C], s_argsAddr, argbias_C);
		s_load_dword(2, s_args[argidx_D], s_argsAddr, argbias_D);
		s_load_dword(2, s_args[argidx_Alpha] ^ 2, s_argsAddr, argbias_Alpha);
		s_load_dword(4, s_args[argidx_StrD0] ^ 4, s_argsAddr, argbias_StrD0);
		
		s_args[argidx_C] = s_args[argidx_C] ^ 1;
		s_args[argidx_D] = s_args[argidx_D] ^ 1;
		s_args[argidx_StrC0] = s_args[argidx_StrC0] ^ 1;
		s_args[argidx_StrD0] = s_args[argidx_StrD0] ^ 1;
		s_args[argidx_BtStrC] = s_args[argidx_BtStrC] ^ 1;
		s_args[argidx_BtStrD] = s_args[argidx_BtStrD] ^ 1;

		s_args[argidx_Alpha] = s_args[argidx_Alpha] ^ 1;
		s_args[argidx_Beta] = s_args[argidx_Beta] ^ 1;

		// ---------------------------------------------------------------------
		addr_at_3();
		addr_bn_3();
		s_wait_lgkmcnt(0);
		addr_cn_1();
		addr_dn(); 

		fetch_c();

		math_loop();

		free_math_gprs();
	}

	// =======================================================================
	bool addr_at_0()
	{
		// ---------------------------------------------------------------------
		a_fetch_instr_dw_sz = en_direct_glb_to_lds ? 1 : 4;
		a_fetch_instr_sz = a_fetch_instr_dw_sz * GPR_SZ;
		d_store_instr_dw_sz = (k_param.DataType == E_DataType::Fp32) ? 4 : 2;
		d_store_instr_sz = d_store_instr_dw_sz * GPR_SZ;

		lds_pad_dw = en_direct_glb_to_lds ? 2 : (k_param.DataType == E_DataType::Fp16) ? 2 : 1;
		lds_pad_byte = lds_pad_dw * GPR_SZ;
		lds_wr_instr_dw_sz = 4;
		lds_wr_instr_sz = lds_wr_instr_dw_sz * GPR_SZ;
		lds_rd_instr_dw_sz = (k_param.DataType == E_DataType::Fp16) ? 2 : 1;
		lds_rd_instr_sz = lds_rd_instr_dw_sz * GPR_SZ;

		uint32_t du_dw_sz = k_param.loop_unroll * elem_sz / GPR_SZ;
		if (du_dw_sz > 32)
		{
			lds_nopad_dw_len = du_dw_sz;
			lds_nopad_row_num = 1;
		}
		else
		{
			lds_nopad_dw_len = 32;
			lds_nopad_row_num = lds_nopad_dw_len / du_dw_sz;
		}
		lds_wr_nopad_trd_num = lds_nopad_dw_len / lds_wr_instr_dw_sz;

		// ---------------------------------------------------------------------
		math_wave_num_per_grp = wv_pttn0_per_grp * wv_pttn1_per_grp;
		elem_num0_per_grp = mfma_m * mfma_pttn0_per_wv * wv_pttn0_per_grp;
		mfma_blk0_per_grp = elem_num0_per_grp / mfma_m;

		a_fetch_ele_num0_per_wv = k_param.loop_unroll;
		a_fetch_wave_shape0 = a_fetch_ele_num0_per_wv * elem_sz / a_fetch_instr_sz;
		a_fetch_wave_shape1 = WAVE_SIZE / a_fetch_wave_shape0;
		a_fetch_ele_num1_per_wv = a_fetch_wave_shape1;
		a_fetch_times = elem_num0_per_grp / math_wave_num_per_grp / a_fetch_ele_num1_per_wv;
		if (a_fetch_times < 1)
			return false;

		// ---------------------------------------------------------------------
		if (en_direct_glb_to_lds)
		{
			a_lds_sz_per_wv_per_time = WAVE_SIZE * GPR_SZ + lds_pad_byte;
			a_lds_sz_per_wv = a_lds_sz_per_wv_per_time * a_fetch_times;
			a_lds_sz_per_grp = a_lds_sz_per_wv * math_wave_num_per_grp;

			a_lds_read_step_1 = mfma_k * elem_sz;
			a_lds_read_step_2 = (mfma_m / a_fetch_ele_num1_per_wv) * a_lds_sz_per_wv_per_time;
		}
		else
		{
			uint32_t pad_times = (elem_num0_per_grp * k_param.loop_unroll) * elem_sz / GPR_SZ / lds_nopad_dw_len;
			a_lds_sz_per_grp = (elem_num0_per_grp * k_param.loop_unroll * elem_sz) + pad_times * lds_pad_byte;

			uint32_t pad_times_per_grp_per_time = lds_wr_instr_dw_sz * WAVE_SIZE * math_wave_num_per_grp / lds_nopad_dw_len;
			a_lds_write_step = (lds_wr_instr_sz * WAVE_SIZE * math_wave_num_per_grp) + pad_times_per_grp_per_time * lds_pad_byte;

			a_lds_read_step_1 = mfma_k * elem_sz;
			uint32_t pad_times_per_mfma_tile = (mfma_m * k_param.loop_unroll) * elem_sz / GPR_SZ / lds_nopad_dw_len;
			a_lds_read_step_2 = (mfma_m * k_param.loop_unroll * elem_sz) + pad_times_per_mfma_tile * lds_pad_byte;
		}

		return true;
	}
	void addr_at_1()
	{
		// ---------------------------------------------------------------------
		s_a_dscp = newSgpr("dscr_a", 4, 4);
		if (k_param.enTensileLayout == true)
		{
			//s_load_dword(2, s_a_dscp ^ 2, s_argsAddr, 8 * 5 + 4 * 0);
		}
		else
		{
			//s_load_dword(2, s_a_dscp ^ 2, s_argsAddr, 8 * 0);
		}
		s_a_dscp = s_a_dscp ^ 1;
		op2("s_mov_b32", s_a_dscp + 0, s_args[argidx_A] + 0);
		op2("s_mov_b32", s_a_dscp + 1, s_args[argidx_A] + 1);
		op2h("s_mov_b32", s_a_dscp + 2, 0x80000000);
		op2h("s_mov_b32", s_a_dscp + 3, 0x00020000);
		s_a_dscp = s_a_dscp ^ 4;

		// ---------------------------------------------------------------------
		s_a_dscp = s_a_dscp ^ 1;
		op3("s_mul_i32", s_tmp1, s_bid_z, s_args[argidx_BtStrA]);
		op3("s_lshl_b32", s_tmp1, s_tmp1, log2Int(elem_sz));
		op3("s_add_u32", s_a_dscp + 0, s_a_dscp + 0, s_tmp1);
		op3("s_addc_u32", s_a_dscp + 1, s_a_dscp + 1, 0);
		s_a_dscp = s_a_dscp ^ 4;

		T_Var s_a_grp_row_id_base = newSgpr("a_grp_row_base");
		op3("s_lshl_b32", s_a_grp_row_id_base, s_bid_x, log2Int(elem_num0_per_grp));

		T_Var v_a_wv_row_id_in_grp = newVgpr("a_wv_row_in_grp");
		if (en_direct_glb_to_lds)
		{
			op3("v_lshlrev_b32", v_a_wv_row_id_in_grp, log2Int(a_fetch_ele_num1_per_wv * a_fetch_times), s_wvid_in_grp);
		}
		else
		{
			op3("v_lshlrev_b32", v_a_wv_row_id_in_grp, log2Int(a_fetch_ele_num1_per_wv), s_wvid_in_grp);
		}

		// ---------------------------------------------------------------------
#if 0
		s_wait_lgkmcnt(0);
		T_Var v_a_preload_offset = newVgpr("2mb_offset");
		op3("v_add_u32", v_a_preload_offset, s_a_grp_row_id_base, v_a_wv_row_id_in_grp);
		op3("v_mul_lo_u32", v_a_preload_offset, s_args[argidx_StrA0], v_a_preload_offset);
		op3("v_lshlrev_b32", v_a_preload_offset, log2Int(elem_sz), v_a_preload_offset);
		buffer_load_dword(1, v_tmp2, v_a_preload_offset, s_a_dscp, 0, false, true, false, 0);
		delVar(v_a_preload_offset);
#endif
		// ---------------------------------------------------------------------
		T_Var v_a_trd_row_id_in_wv = newVgpr("a_trd_row_id_in_wv");
		T_Var v_a_trd_col_id_in_wv = newVgpr("a_trd_col_id_in_wv");
		op3("v_lshrrev_b32", v_a_trd_row_id_in_wv, log2Int(a_fetch_wave_shape0), v_tid_in_wave);
		op3("v_and_b32", v_a_trd_col_id_in_wv, a_fetch_wave_shape0 - 1, v_tid_in_wave);

		T_Var v_a_trd_row_id = newVgpr("a_trd_row_id");
		T_Var v_a_row_base_addr = newVgpr("s_a_row_base_addr");
		T_Var v_a_col_offset_addr = newVgpr("s_a_col_offset_addr");
		op4("v_add3_u32", v_a_trd_row_id, s_a_grp_row_id_base, v_a_wv_row_id_in_grp, v_a_trd_row_id_in_wv);
		op3("v_mul_lo_u32", v_tmp1, s_args[argidx_StrA0], v_a_trd_row_id);
		op3("v_lshlrev_b32", v_a_row_base_addr, log2Int(elem_sz), v_tmp1);
		op3("v_lshlrev_b32", v_a_col_offset_addr, log2Int(a_fetch_instr_sz), v_a_trd_col_id_in_wv);

		T_Var s_a_addr_step = newSgpr("a_offset");
		if (en_direct_glb_to_lds)
		{
			op3("s_lshl_b32", s_a_addr_step, s_args[argidx_StrA0], log2Int(a_fetch_ele_num1_per_wv) + log2Int(elem_sz));
			op3("s_sub_u32", s_a_addr_step, s_a_addr_step, a_lds_sz_per_wv_per_time);
		}
		else
		{
			op3("s_lshl_b32", s_a_addr_step, s_args[argidx_StrA0], log2Int(a_fetch_ele_num1_per_wv * math_wave_num_per_grp) + log2Int(elem_sz));
		}

		v_a_fetch_offset = newVgpr("a_trd_row_id_in_wv", a_fetch_times);
		op3("v_add_u32", v_a_fetch_offset, v_a_row_base_addr, v_a_col_offset_addr);
		for (uint32_t i = 0; i < a_fetch_times - 1; i++)
			op3("v_add_u32", v_a_fetch_offset + (i + 1), s_a_addr_step, v_a_fetch_offset + i);

		if (en_direct_glb_to_lds == false)
			v_glb_load_a = newVgpr("glb_a", a_fetch_times*a_fetch_instr_sz, a_fetch_instr_sz);

		delVar(s_a_grp_row_id_base);
		delVar(v_a_wv_row_id_in_grp);
		delVar(v_a_trd_row_id_in_wv);
		delVar(v_a_trd_col_id_in_wv);
		delVar(v_a_row_base_addr);
		delVar(v_a_col_offset_addr);
		delVar(v_a_trd_row_id);
		delVar(s_a_addr_step);
	}
	void addr_at_2()
	{
		ldsAllocByte(a_lds_sz_per_grp * k_param.lds_buffer_num);

		if (en_direct_glb_to_lds)
		{
			if (k_param.lds_buffer_num == 2)
			{
				s_a_lds_write = newSgpr("a_lds_write");
				s_a_lds_write_0 = newSgpr("a_lds_addr_ping");
				s_a_lds_write_1 = newSgpr("a_lds_addr_pang");
				s_a_lds_write_2 = newSgpr("a_lds_addr_pang");
				op2("s_mov_b32", s_tmp1, a_lds_sz_per_wv);
				op3("s_mul_i32", s_a_lds_write_0, s_wvid_in_grp, s_tmp1);
				op3("s_add_i32", s_a_lds_write_1, s_a_lds_write_0, a_lds_sz_per_grp);
				op3("s_xor_b32", s_a_lds_write_2, s_a_lds_write_0, s_a_lds_write_1);

				op2("s_mov_b32", s_a_lds_write, s_a_lds_write_0);
			}
			else if (k_param.lds_buffer_num >= 3)
			{
				s_a_lds_write = newSgpr("a_lds_write");
				s_a_lds_write_0 = newSgpr("a_lds_addr_ping");
				s_a_lds_write_1 = newSgpr("a_lds_addr_pang");
				s_a_lds_write_2 = newSgpr("a_lds_addr_pang");
				op2("s_mov_b32", s_tmp1, a_lds_sz_per_wv);
				op3("s_mul_i32", s_a_lds_write_0, s_wvid_in_grp, s_tmp1);

				op2("s_mov_b32", s_a_lds_write_2, a_lds_sz_per_grp);
				op2("s_mov_b32", s_a_lds_write, s_a_lds_write_0);
			}
		}
		else
		{
			if (k_param.lds_buffer_num == 2)
			{
				v_a_lds_write = newVgpr("a_lds_write");
				v_a_lds_write_0 = newVgpr("a_lds_addr_ping");
				v_a_lds_write_1 = newVgpr("a_lds_addr_pang");
				v_a_lds_write_2 = newVgpr("a_lds_addr_exch");
				op3("v_lshrrev_b32", v_tmp1, log2Int(lds_wr_nopad_trd_num), v_tid_x);
				op3("v_lshlrev_b32", v_tmp1, log2Int(lds_pad_byte), v_tmp1);
				op3("v_lshlrev_b32", v_a_lds_write_0, log2Int(lds_wr_instr_sz), v_tid_x);
				op3("v_add_u32", v_a_lds_write_0, v_a_lds_write_0, v_tmp1);
				op2("v_mov_b32", v_a_lds_write, v_a_lds_write_0);

				op2("v_mov_b32", v_tmp1, a_lds_sz_per_grp);
				op3("v_add_u32", v_a_lds_write_1, v_a_lds_write_0, v_tmp1);

				op3("v_xor_b32", v_a_lds_write_2, v_a_lds_write_0, v_a_lds_write_1);
			}
			else
			{
				v_a_lds_write = newVgpr("a_lds_write");
				v_a_lds_write_0 = newVgpr("a_lds_addr_ping");
				v_a_lds_write_1 = newVgpr("a_lds_addr_tmp");
				v_a_lds_write_2 = newVgpr("a_lds_addr_size");
				op3("v_lshrrev_b32", v_tmp1, log2Int(lds_wr_nopad_trd_num), v_tid_x);
				op3("v_lshlrev_b32", v_tmp1, log2Int(lds_pad_byte), v_tmp1);
				op3("v_lshlrev_b32", v_a_lds_write_0, log2Int(lds_wr_instr_sz), v_tid_x);
				op3("v_add_u32", v_a_lds_write_0, v_a_lds_write_0, v_tmp1);
				op2("v_mov_b32", v_a_lds_write_2, a_lds_sz_per_grp);
				op2("v_mov_b32", v_a_lds_write, v_a_lds_write_0);
			}
		}
	}
	void addr_at_3()
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
		op3("v_lshlrev_b32", v_tmp1, log2Int(k_param.loop_unroll), v_a_trd_row_id); // row_id_in_mfma * depthU
		op3("v_lshlrev_b32", v_a_row_base_addr, log2Int(elem_sz), v_tmp1);
		op3("v_lshlrev_b32", v_a_col_offset_addr, log2Int(lds_rd_instr_sz), v_a_trd_col_id_in_mfma);

		// lds pad offset (byte)
		T_Var v_a_lds_pad_offset = newVgpr("mfma_col_id");
		if (en_direct_glb_to_lds)
		{
			op3("v_lshrrev_b32", v_tmp1, log2Int(a_fetch_ele_num1_per_wv), v_a_trd_row_id);
			op3("v_mul_lo_u32", v_a_lds_pad_offset, lds_pad_byte, v_tmp1);
		}
		else
		{
			op3("v_lshrrev_b32", v_tmp1, log2Int(lds_nopad_row_num), v_a_trd_row_id);
			op3("v_lshlrev_b32", v_a_lds_pad_offset, log2Int(lds_pad_byte), v_tmp1);
		}

		// ---------------------------------------------------------------------
		if (k_param.lds_buffer_num == 2)
		{
			v_a_lds_read = newVgpr("a_lds_offset");
			v_a_lds_read_0 = newVgpr("a_lds_offset");
			v_a_lds_read_1 = newVgpr("a_lds_offset");
			v_a_lds_read_2 = newVgpr("a_lds_exchange"); // for exchange
			op3("v_add_u32", v_a_lds_read_0, v_a_row_base_addr, v_a_col_offset_addr);
			op3("v_add_u32", v_a_lds_read_0, v_a_lds_read_0, v_a_lds_pad_offset);
			op3("v_add_u32", v_a_lds_read_1, a_lds_sz_per_grp, v_a_lds_read_0);
			op3("v_xor_b32", v_a_lds_read_2, v_a_lds_read_1, v_a_lds_read_0);
			op2("v_mov_b32", v_a_lds_read, v_a_lds_read_0);
		}
		else if (k_param.lds_buffer_num >= 3)
		{
			v_a_lds_read = newVgpr("a_lds_offset");
			v_a_lds_read_0 = newVgpr("a_lds_offset");
			v_a_lds_read_1 = newVgpr("a_lds_offset");
			v_a_lds_read_2 = newVgpr("a_lds_size"); // for lds size 
			op3("v_add_u32", v_a_lds_read_0, v_a_row_base_addr, v_a_col_offset_addr);
			op3("v_add_u32", v_a_lds_read_0, v_a_lds_read_0, v_a_lds_pad_offset);
			op2("v_mov_b32", v_a_lds_read_2, a_lds_sz_per_grp);
			op2("v_mov_b32", v_a_lds_read, v_a_lds_read_0);
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
	
	bool addr_bn_0()
	{
		// ---------------------------------------------------------------------
		b_fetch_instr_dw_sz = en_direct_glb_to_lds ? 1 : 4;
		b_fetch_instr_sz = b_fetch_instr_dw_sz * GPR_SZ;

		lds_pad_dw = en_direct_glb_to_lds ? 2 : (k_param.DataType == E_DataType::Fp16) ? 2 : 1;
		lds_pad_byte = lds_pad_dw * GPR_SZ;
		lds_wr_instr_dw_sz = 4;
		lds_wr_instr_sz = lds_wr_instr_dw_sz * GPR_SZ;
		lds_rd_instr_dw_sz = (k_param.DataType == E_DataType::Fp16) ? 2 : 1;
		lds_rd_instr_sz = lds_rd_instr_dw_sz * GPR_SZ;

		uint32_t du_dw_sz = k_param.loop_unroll * elem_sz / GPR_SZ;
		if (du_dw_sz > 32)
		{
			lds_nopad_dw_len = du_dw_sz;
			lds_nopad_row_num = 1;
		}
		else
		{
			lds_nopad_dw_len = 32;
			lds_nopad_row_num = lds_nopad_dw_len / du_dw_sz;
		}
		lds_wr_nopad_trd_num = lds_nopad_dw_len / lds_wr_instr_dw_sz;

		// ---------------------------------------------------------------------
		math_wave_num_per_grp = wv_pttn0_per_grp * wv_pttn1_per_grp;
		elem_num1_per_grp = mfma_n * mfma_pttn1_per_wv * wv_pttn1_per_grp;
		mfma_blk1_per_grp = elem_num1_per_grp / mfma_n;

		b_fetch_ele_num0_per_wv = k_param.loop_unroll;
		b_fetch_wave_shape0 = b_fetch_ele_num0_per_wv * elem_sz / b_fetch_instr_sz;
		b_fetch_wave_shape1 = WAVE_SIZE / b_fetch_wave_shape0;
		b_fetch_ele_num1_per_wv = b_fetch_wave_shape1;
		b_fetch_times = elem_num1_per_grp / math_wave_num_per_grp / b_fetch_ele_num1_per_wv;
		if (b_fetch_times < 1)
			return false;

		// ---------------------------------------------------------------------
		if (en_direct_glb_to_lds)
		{
			b_lds_sz_per_wv_per_time = WAVE_SIZE * GPR_SZ + lds_pad_byte;
			b_lds_sz_per_wv = b_lds_sz_per_wv_per_time * b_fetch_times;
			b_lds_sz_per_grp = b_lds_sz_per_wv * math_wave_num_per_grp;

			b_lds_read_step_1 = mfma_k * elem_sz;
			b_lds_read_step_2 = (mfma_n / b_fetch_ele_num1_per_wv) * b_lds_sz_per_wv_per_time;
		}
		else
		{
			uint32_t pad_times = (elem_num1_per_grp * k_param.loop_unroll) * elem_sz / GPR_SZ / lds_nopad_dw_len;
			b_lds_sz_per_grp = (elem_num1_per_grp * k_param.loop_unroll * elem_sz) + pad_times * lds_pad_byte;

			uint32_t pad_times_per_grp_per_time = lds_wr_instr_dw_sz * WAVE_SIZE * math_wave_num_per_grp / lds_nopad_dw_len;
			b_lds_write_step = (lds_wr_instr_sz * WAVE_SIZE * math_wave_num_per_grp) + pad_times_per_grp_per_time * lds_pad_byte;

			b_lds_read_step_1 = mfma_k * elem_sz;
			uint32_t pad_times_per_mfma_tile = (mfma_n * k_param.loop_unroll) * elem_sz / GPR_SZ / lds_nopad_dw_len;
			b_lds_read_step_2 = (mfma_n * k_param.loop_unroll * elem_sz) + pad_times_per_mfma_tile * lds_pad_byte;
		}

		return true;
	}
	void addr_bn_1()
	{
		// ---------------------------------------------------------------------
		s_b_dscp = newSgpr("dscr_b", 4, 4);
		if (k_param.enTensileLayout == true)
		{
			//s_load_dword(2, s_b_dscp ^ 2, s_argsAddr ^ 2, 8 * 6 + 4 * 0);
		}
		else
		{
			//s_load_dword(2, s_b_dscp ^ 2, s_argsAddr, 8 * 1);
		}
		s_b_dscp = s_b_dscp ^ 1;
		op2("s_mov_b32", s_b_dscp + 0, s_args[argidx_B] + 0);
		op2("s_mov_b32", s_b_dscp + 1, s_args[argidx_B] + 1);
		op2h("s_mov_b32", s_b_dscp + 2, 0x80000000);
		op2h("s_mov_b32", s_b_dscp + 3, 0x00020000);
		s_b_dscp = s_b_dscp ^ 4;

		// ---------------------------------------------------------------------
		s_b_dscp = s_b_dscp ^ 1;
		op3("s_mul_i32", s_tmp1, s_bid_z, s_args[argidx_BtStrB]);
		op3("s_lshl_b32", s_tmp1, s_tmp1, log2Int(elem_sz));
		op3("s_add_u32", s_b_dscp + 0, s_b_dscp + 0, s_tmp1);
		op3("s_addc_u32", s_b_dscp + 1, s_b_dscp + 1, 0);
		s_b_dscp = s_b_dscp ^ 4;

		T_Var s_b_grp_row_id_base = newSgpr("b_grp_row_base");
		op3("s_lshl_b32", s_b_grp_row_id_base, s_bid_y, log2Int(elem_num1_per_grp));

		T_Var v_b_wv_row_id_in_grp = newVgpr("b_wv_row_in_grp");
		if (en_direct_glb_to_lds)
		{
			op3("v_lshlrev_b32", v_b_wv_row_id_in_grp, log2Int(b_fetch_ele_num1_per_wv * b_fetch_times), s_wvid_in_grp);
		}
		else
		{
			op3("v_lshlrev_b32", v_b_wv_row_id_in_grp, log2Int(b_fetch_ele_num1_per_wv), s_wvid_in_grp);
		}

		// ---------------------------------------------------------------------
#if 0
		s_wait_lgkmcnt(0);
		T_Var v_b_preload_offset = newVgpr("2mb_offsetb");
		op3("v_add_u32", v_b_preload_offset, s_b_grp_row_id_base, v_b_wv_row_id_in_grp);
		op3("v_mul_lo_u32", v_b_preload_offset, s_args[argidx_StrB0], v_b_preload_offset);
		op3("v_lshlrev_b32", v_b_preload_offset, log2Int(elem_sz), v_b_preload_offset);
		buffer_load_dword(1, v_tmp2, v_b_preload_offset, s_b_dscp, 0, false, true, false, 0);
		delVar(v_b_preload_offset);
#endif
		// ---------------------------------------------------------------------
		T_Var v_b_trd_row_id_in_wv = newVgpr("b_trd_row_id_in_wv");
		T_Var v_b_trd_col_id_in_wv = newVgpr("b_trd_col_id_in_wv");
		op3("v_lshrrev_b32", v_b_trd_row_id_in_wv, log2Int(b_fetch_wave_shape0), v_tid_in_wave);
		op3("v_and_b32", v_b_trd_col_id_in_wv, b_fetch_wave_shape0 - 1, v_tid_in_wave);

		T_Var v_b_trd_row_id = newVgpr("b_trd_row_id");
		T_Var v_b_row_base_addr = newVgpr("s_b_row_base_addr");
		T_Var v_b_col_offset_addr = newVgpr("s_b_col_offset_addr");
		op4("v_add3_u32", v_b_trd_row_id, s_b_grp_row_id_base, v_b_wv_row_id_in_grp, v_b_trd_row_id_in_wv);
		op3("v_mul_lo_u32", v_tmp1, s_args[argidx_StrB0], v_b_trd_row_id);
		op3("v_lshlrev_b32", v_b_row_base_addr, log2Int(elem_sz), v_tmp1);
		op3("v_lshlrev_b32", v_b_col_offset_addr, log2Int(b_fetch_instr_sz), v_b_trd_col_id_in_wv);

		T_Var s_b_addr_step = newSgpr("b_offset");
		if (en_direct_glb_to_lds)
		{
			op3("s_lshl_b32", s_b_addr_step, s_args[argidx_StrB0], log2Int(b_fetch_ele_num1_per_wv) + log2Int(elem_sz));
			op3("s_sub_u32", s_b_addr_step, s_b_addr_step, b_lds_sz_per_wv_per_time);
		}
		else
		{
			op3("s_lshl_b32", s_b_addr_step, s_args[argidx_StrB0], log2Int(b_fetch_ele_num1_per_wv * math_wave_num_per_grp) + log2Int(elem_sz));
		}

		v_b_fetch_offset = newVgpr("b_trd_row_id_in_wv", b_fetch_times);
		op3("v_add_u32", v_b_fetch_offset, v_b_row_base_addr, v_b_col_offset_addr);
		for (uint32_t i = 0; i < b_fetch_times - 1; i++)
			op3("v_add_u32", v_b_fetch_offset + (i + 1), s_b_addr_step, v_b_fetch_offset + i);

		if (en_direct_glb_to_lds == false)
			v_glb_load_b = newVgpr("glb_b", b_fetch_times*b_fetch_instr_sz, b_fetch_instr_sz);

		delVar(s_b_grp_row_id_base);
		delVar(v_b_wv_row_id_in_grp);
		delVar(v_b_trd_row_id_in_wv);
		delVar(v_b_trd_col_id_in_wv);
		delVar(v_b_trd_row_id);
		delVar(v_b_row_base_addr);
		delVar(v_b_col_offset_addr);
		delVar(s_b_addr_step);
	}
	void addr_bn_2()
	{
		ldsAllocByte(b_lds_sz_per_grp * k_param.lds_buffer_num);

		if (en_direct_glb_to_lds)
		{
			if (k_param.lds_buffer_num == 2)
			{
				s_b_lds_write = newSgpr("b_lds_addr_ping");
				s_b_lds_write_0 = newSgpr("b_lds_addr_ping");
				s_b_lds_write_1 = newSgpr("b_lds_addr_pang");
				s_b_lds_write_2 = newSgpr("b_lds_addr_pang");
				op2("s_mov_b32", s_tmp1, b_lds_sz_per_wv);
				op3("s_mul_i32", s_b_lds_write_0, s_wvid_in_grp, s_tmp1);
				op2("s_mov_b32", s_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
				op3("s_add_u32", s_b_lds_write_0, s_b_lds_write_0, s_tmp1);
				op3("s_add_i32", s_b_lds_write_1, s_b_lds_write_0, b_lds_sz_per_grp);
				op3("s_xor_b32", s_b_lds_write_2, s_b_lds_write_0, s_b_lds_write_1);
				op2("s_mov_b32", s_b_lds_write, s_b_lds_write_0);
			}
			else if (k_param.lds_buffer_num >= 3)
			{
				s_b_lds_write = newSgpr("b_lds_addr_ping");
				s_b_lds_write_0 = newSgpr("b_lds_addr_ping");
				s_b_lds_write_1 = newSgpr("b_lds_addr_pang");
				s_b_lds_write_2 = newSgpr("b_lds_addr_pang");
				op2("s_mov_b32", s_tmp1, b_lds_sz_per_wv);
				op3("s_mul_i32", s_b_lds_write_0, s_wvid_in_grp, s_tmp1);
				op2("s_mov_b32", s_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
				op3("s_add_u32", s_b_lds_write_0, s_b_lds_write_0, s_tmp1);
				op2("s_mov_b32", s_b_lds_write_2, b_lds_sz_per_grp);
				op2("s_mov_b32", s_b_lds_write, s_b_lds_write_0);
			}
		}
		else
		{
			if (k_param.lds_buffer_num == 2)
			{
				v_b_lds_write = newVgpr("b_lds_write");
				v_b_lds_write_0 = newVgpr("b_lds_addr_ping");
				v_b_lds_write_1 = newVgpr("b_lds_addr_pang");
				v_b_lds_write_2 = newVgpr("b_lds_addr_exch");
				op3("v_lshrrev_b32", v_tmp1, log2Int(lds_wr_nopad_trd_num), v_tid_x);
				op3("v_lshlrev_b32", v_tmp1, log2Int(lds_pad_byte), v_tmp1);
				op3("v_lshlrev_b32", v_b_lds_write_0, log2Int(lds_wr_instr_sz), v_tid_x);
				op3("v_add_u32", v_b_lds_write_0, v_b_lds_write_0, v_tmp1);
				op2("v_mov_b32", v_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
				op3("v_add_u32", v_b_lds_write_0, v_b_lds_write_0, v_tmp1);
				op2("v_mov_b32", v_b_lds_write, v_b_lds_write_0);

				op2("v_mov_b32", v_tmp1, b_lds_sz_per_grp);
				op3("v_add_u32", v_b_lds_write_1, v_b_lds_write_0, v_tmp1);

				op3("v_xor_b32", v_b_lds_write_2, v_b_lds_write_0, v_b_lds_write_1);
			}
			else
			{
				v_b_lds_write = newVgpr("b_lds_write");
				v_b_lds_write_0 = newVgpr("b_lds_addr_ping");
				v_b_lds_write_1 = newVgpr("b_lds_addr_tmp");
				v_b_lds_write_2 = newVgpr("b_lds_addr_size");
				op3("v_lshrrev_b32", v_tmp1, log2Int(lds_wr_nopad_trd_num), v_tid_x);
				op3("v_lshlrev_b32", v_tmp1, log2Int(lds_pad_byte), v_tmp1);
				op3("v_lshlrev_b32", v_b_lds_write_0, log2Int(lds_wr_instr_sz), v_tid_x);
				op3("v_add_u32", v_b_lds_write_0, v_b_lds_write_0, v_tmp1);
				op2("v_mov_b32", v_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
				op3("v_add_u32", v_b_lds_write_0, v_b_lds_write_0, v_tmp1);
				op2("v_mov_b32", v_b_lds_write_2, b_lds_sz_per_grp);
				op2("v_mov_b32", v_b_lds_write, v_b_lds_write_0);
			}
		}
	}
	void addr_bn_3()
	{
		// ---------------------------------------------------------------------
		T_Var v_b_wv_col_id_in_wvpttn = newVgpr("mfma_col_id");
		T_Var v_b_wv_row_id_in_grp = newVgpr("mfma_row_id");
		op3("v_lshrrev_b32", v_b_wv_col_id_in_wvpttn, log2Int(wv_pttn0_per_grp), s_wvid_in_grp);
		op3("v_lshlrev_b32", v_b_wv_row_id_in_grp, log2Int(mfma_sz1_per_wv), v_b_wv_col_id_in_wvpttn);

		T_Var v_b_trd_row_id_in_mfma = newVgpr("row_id_in_mt");
		T_Var v_b_trd_col_id_in_mfma = newVgpr("col_id_in_mt");
		op3("v_and_b32", v_b_trd_row_id_in_mfma, v_tid_in_wave, mfma_m - 1);
		op3("v_lshrrev_b32", v_b_trd_col_id_in_mfma, log2Int(mfma_m), v_tid_in_wave);

		T_Var v_b_trd_row_id = newVgpr("row_id_in_mt");
		T_Var v_b_row_base_addr = newVgpr("s_b_row_base_addr");
		T_Var v_b_col_offset_addr = newVgpr("s_b_col_offset_addr");
		op3("v_add_u32", v_b_trd_row_id, v_b_wv_row_id_in_grp, v_b_trd_row_id_in_mfma);
		op3("v_lshlrev_b32", v_tmp1, log2Int(k_param.loop_unroll), v_b_trd_row_id);
		op3("v_lshlrev_b32", v_b_row_base_addr, log2Int(elem_sz), v_tmp1);
		op3("v_lshlrev_b32", v_b_col_offset_addr, log2Int(lds_rd_instr_sz), v_b_trd_col_id_in_mfma);

		T_Var v_b_lds_pad_offset = newVgpr("mfma_col_id");
		if (en_direct_glb_to_lds)
		{
			op3("v_lshrrev_b32", v_tmp1, log2Int(b_fetch_ele_num1_per_wv), v_b_trd_row_id);
			op3("v_mul_lo_u32", v_b_lds_pad_offset, lds_pad_byte, v_tmp1);
		}
		else
		{
			op3("v_lshrrev_b32", v_tmp1, log2Int(lds_nopad_row_num), v_b_trd_row_id);
			op3("v_lshlrev_b32", v_b_lds_pad_offset, log2Int(lds_pad_byte), v_tmp1);
		}

		// ---------------------------------------------------------------------
		if (k_param.lds_buffer_num == 2)
		{
			v_b_lds_read = newVgpr("a_lds_offset");
			v_b_lds_read_0 = newVgpr("a_lds_offset");
			v_b_lds_read_1 = newVgpr("a_lds_offset");
			v_b_lds_read_2 = newVgpr("a_lds_offset");
			op3("v_add_u32", v_b_lds_read_0, v_b_row_base_addr, v_b_col_offset_addr);
			op2h("v_mov_b32", v_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
			op3("v_add_u32", v_b_lds_read_0, v_b_lds_read_0, v_tmp1);
			op3("v_add_u32", v_b_lds_read_0, v_b_lds_read_0, v_b_lds_pad_offset);
			op3("v_add_u32", v_b_lds_read_1, b_lds_sz_per_grp, v_b_lds_read_0);
			op3("v_xor_b32", v_b_lds_read_2, v_b_lds_read_1, v_b_lds_read_0);
			op2("v_mov_b32", v_b_lds_read, v_b_lds_read_0);
		}
		else if (k_param.lds_buffer_num >= 3)
		{
			v_b_lds_read = newVgpr("a_lds_offset");
			v_b_lds_read_0 = newVgpr("a_lds_offset");
			v_b_lds_read_1 = newVgpr("a_lds_offset");
			v_b_lds_read_2 = newVgpr("a_lds_offset");
			op3("v_add_u32", v_b_lds_read_0, v_b_row_base_addr, v_b_col_offset_addr);
			op2h("v_mov_b32", v_tmp1, a_lds_sz_per_grp * k_param.lds_buffer_num);
			op3("v_add_u32", v_b_lds_read_0, v_b_lds_read_0, v_tmp1);
			op3("v_add_u32", v_b_lds_read_0, v_b_lds_read_0, v_b_lds_pad_offset);
			op2("v_mov_b32", v_b_lds_read_2, b_lds_sz_per_grp);
			op2("v_mov_b32", v_b_lds_read, v_b_lds_read_0);
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
	
	bool addr_cn_0()
	{
		if (k_param.DataType == E_DataType::Fp32)c_fetch_instr_dw_sz = 4;
		if (k_param.DataType == E_DataType::Fp16)c_fetch_instr_dw_sz = 2;
		if (k_param.DataType == E_DataType::Bf16)c_fetch_instr_dw_sz = 4;

		c_fetch_instr_sz = c_fetch_instr_dw_sz * GPR_SZ;

		return true;
	}
	void addr_cn_1()
	{
		// ---------------------------------------------------------------------
		s_c_dscp = newSgpr("dscr_c", 4, 4);
		if (k_param.enTensileLayout == true)
		{
			//s_load_dword(2, s_b_dscp ^ 2, s_argsAddr ^ 2, 8 * 6 + 4 * 0);
		}
		else
		{
			//s_load_dword(2, s_c_dscp ^ 2, s_argsAddr, 8 * 2);
		}
		s_c_dscp = s_c_dscp ^ 1;
		op2("s_mov_b32", s_c_dscp + 0, s_args[argidx_C] + 0);
		op2("s_mov_b32", s_c_dscp + 1, s_args[argidx_C] + 1);
		op2h("s_mov_b32", s_c_dscp + 2, 0x80000000);
		op2h("s_mov_b32", s_c_dscp + 3, 0x00020000);
		s_c_dscp = s_c_dscp ^ 4;

		// -----------------------------------------------------------------------
		s_c_dscp = s_c_dscp ^ 1;
		op3("s_mul_i32", s_tmp1, s_bid_z, s_args[argidx_BtStrC]);
		op3("s_lshl_b32", s_tmp1, s_tmp1, log2Int(c_elem_sz));
		op3("s_add_u32", s_c_dscp + 0, s_c_dscp + 0, s_tmp1);
		op3("s_addc_u32", s_c_dscp + 1, s_c_dscp + 1, 0);
		s_c_dscp = s_c_dscp ^ 4;

		T_Var s_grp_row_id_base = newSgpr("grp_row_base");
		T_Var s_grp_col_id_base = newSgpr("col_grp_base");
		op3("s_lshl_b32", s_grp_col_id_base, s_bid_x, log2Int(elem_num0_per_grp * c_elem_sz / GPR_SZ));
		op3("s_lshl_b32", s_grp_row_id_base, s_bid_y, log2Int(elem_num1_per_grp));

		T_Var v_wv_row_id_in_grp = newVgpr("wv_row_in_grp");
		T_Var v_wv_col_id_in_grp = newVgpr("wv_col_in_grp");
		op3("s_lshr_b32", s_tmp1, s_wvid_in_grp, log2Int(wv_pttn0_per_grp));
		op3("v_lshlrev_b32", v_wv_row_id_in_grp, log2Int(mfma_sz1_per_wv), s_tmp1);
		op3("s_and_b32", s_tmp1, s_wvid_in_grp, wv_pttn0_per_grp - 1);
		op3("v_lshlrev_b32", v_wv_col_id_in_grp, log2Int(mfma_sz0_per_wv * c_elem_sz / GPR_SZ), s_tmp1);

		T_Var v_trd_row_id_in_mfma = newVgpr("trd_row_id_in_mfma");
		T_Var v_trd_lane_id_in_mfma = newVgpr("trd_lane_id_in_mfma");
		T_Var v_trd_col_id_in_mfma = newVgpr("trd_col_id_in_mfma");
		op3("v_and_b32", v_trd_row_id_in_mfma, mfma_m - 1, v_tid_in_wave);
		op3("v_lshrrev_b32", v_trd_lane_id_in_mfma, log2Int(mfma_m), v_tid_in_wave);
		op3("v_lshlrev_b32", v_trd_col_id_in_mfma, log2Int(mfma_lane_shape_m * c_elem_sz / GPR_SZ), v_trd_lane_id_in_mfma);

		T_Var v_trd_row_idx = newVgpr("trd_row_idx");
		T_Var v_trd_col_idx = newVgpr("trd_col_idx");
		op4("v_add3_u32", v_trd_row_idx, s_grp_row_id_base, v_wv_row_id_in_grp, v_trd_row_id_in_mfma);
		op4("v_add3_u32", v_trd_col_idx, s_grp_col_id_base, v_wv_col_id_in_grp, v_trd_col_id_in_mfma);

		T_Var v_row_base_addr = newVgpr("row_base_addr");
		T_Var v_col_offset_addr = newVgpr("col_offset_addr");
		op3("v_mul_lo_u32", v_tmp1, s_args[argidx_StrC0], v_trd_row_idx);
		op3("v_lshlrev_b32", v_row_base_addr, log2Int(c_elem_sz), v_tmp1);
		op3("v_lshlrev_b32", v_col_offset_addr, log2Int(GPR_SZ), v_trd_col_idx);

		T_Var v_base_addr = newVgpr("base_address");
		op3("v_add_u32", v_base_addr, v_row_base_addr, v_col_offset_addr);

		// -----------------------------------------------------------------------
		c_glb_step1 = c_elem_sz * mfma_lane_shape_m * mfma_lane_num_per_tile;
		c_glb_step2 = c_elem_sz * mfma_m;
		T_Var v_c_glb_step3 = newVgpr("c_glb_step3");
		op3("v_lshlrev_b32", v_tmp1, log2Int(mfma_n), s_args[argidx_StrC0]);
		op3("v_lshlrev_b32", v_c_glb_step3, log2Int(c_elem_sz), v_tmp1);

		v_c_fetch_offset = newVgpr("c_store_offset", mfma_pttn1_per_wv);
		op2("v_mov_b32", v_c_fetch_offset, v_base_addr);
		for (uint32_t i = 0; i < mfma_pttn1_per_wv - 1; i++)
			op3("v_add_u32", v_c_fetch_offset + (i + 1), v_c_glb_step3, v_c_fetch_offset + i);

		// -----------------------------------------------------------------------
		v_glb_load_c = newVgpr("glb_c", mfma_dgpr_per_mfma * mfma_pttn0_per_wv * mfma_pttn1_per_wv, c_fetch_instr_dw_sz);

		delVar(s_grp_row_id_base);
		delVar(s_grp_col_id_base);
		delVar(v_wv_row_id_in_grp);
		delVar(v_wv_col_id_in_grp);
		delVar(v_trd_row_id_in_mfma);
		delVar(v_trd_lane_id_in_mfma);
		delVar(v_trd_col_id_in_mfma);
		delVar(v_trd_row_idx);
		delVar(v_trd_col_idx);
		delVar(v_row_base_addr);
		delVar(v_col_offset_addr);
		delVar(v_base_addr);
		delVar(v_c_glb_step3);
	}
	
	bool addr_dn_0()
	{
		d_store_instr_dw_sz = (k_param.DataType == E_DataType::Fp32) ? 4 : 2;
		d_store_instr_sz = d_store_instr_dw_sz * GPR_SZ;

		return true;
	}
	void addr_dn()
	{
		// -----------------------------------------------------------------------
		s_d_dscp = newSgpr("dscr_d", 4, 4);
		if (k_param.enTensileLayout == true)
		{
			//s_load_dword(2, s_d_dscp ^ 2, s_argsAddr ^ 2, 8 * 3 + 4 * 0);
		}
		else
		{
			//s_load_dword(2, s_d_dscp ^ 2, s_argsAddr, 8 * 3);
		}
		s_d_dscp = s_d_dscp ^ 1;
		op2("s_mov_b32", s_d_dscp + 0, s_args[argidx_D] + 0);
		op2("s_mov_b32", s_d_dscp + 1, s_args[argidx_D] + 1);
		op2h("s_mov_b32", s_d_dscp + 2, 0x80000000);
		op2h("s_mov_b32", s_d_dscp + 3, 0x00020000);
		s_d_dscp = s_d_dscp ^ 4;

		// -----------------------------------------------------------------------
		s_d_dscp = s_d_dscp ^ 1;
		op3("s_mul_i32", s_tmp1, s_bid_z, s_args[argidx_BtStrD]);
		op3("s_lshl_b32", s_tmp1, s_tmp1, log2Int(elem_sz));
		op3("s_add_u32", s_d_dscp + 0, s_d_dscp + 0, s_tmp1);
		op3("s_addc_u32", s_d_dscp + 1, s_d_dscp + 1, 0);
		s_d_dscp = s_d_dscp ^ 4;

		T_Var s_d_grp_row_id_base = newSgpr("d_grp_row_base");
		T_Var s_d_grp_col_id_base = newSgpr("d_col_grp_base");
		op3("s_lshl_b32", s_d_grp_col_id_base, s_bid_x, log2Int(elem_num0_per_grp * elem_sz / GPR_SZ));
		op3("s_lshl_b32", s_d_grp_row_id_base, s_bid_y, log2Int(elem_num1_per_grp));

		T_Var v_d_wv_row_id_in_grp = newVgpr("d_wv_row_in_grp");
		T_Var v_d_wv_col_id_in_grp = newVgpr("d_wv_col_in_grp");
		op3("s_lshr_b32", s_tmp1, s_wvid_in_grp, log2Int(wv_pttn0_per_grp));
		op3("v_lshlrev_b32", v_d_wv_row_id_in_grp, log2Int(mfma_sz1_per_wv), s_tmp1);
		op3("s_and_b32", s_tmp1, s_wvid_in_grp, wv_pttn0_per_grp - 1);
		op3("v_lshlrev_b32", v_d_wv_col_id_in_grp, log2Int(mfma_sz0_per_wv * elem_sz / GPR_SZ), s_tmp1);

		T_Var v_d_trd_row_id_in_mfma = newVgpr("d_trd_row_id_in_mfma");
		T_Var v_d_trd_lane_id_in_mfma = newVgpr("d_trd_lane_id_in_mfma");
		T_Var v_d_trd_col_id_in_mfma = newVgpr("d_trd_col_id_in_mfma");
		op3("v_and_b32", v_d_trd_row_id_in_mfma, mfma_m - 1, v_tid_in_wave);
		op3("v_lshrrev_b32", v_d_trd_lane_id_in_mfma, log2Int(mfma_m), v_tid_in_wave);
		op3("v_lshlrev_b32", v_d_trd_col_id_in_mfma, log2Int(mfma_lane_shape_m * elem_sz / GPR_SZ), v_d_trd_lane_id_in_mfma);

		T_Var v_d_trd_row_idx = newVgpr("d_trd_row_idx");
		T_Var v_d_trd_col_idx = newVgpr("d_trd_col_idx");
		op4("v_add3_u32", v_d_trd_row_idx, s_d_grp_row_id_base, v_d_wv_row_id_in_grp, v_d_trd_row_id_in_mfma);
		op4("v_add3_u32", v_d_trd_col_idx, s_d_grp_col_id_base, v_d_wv_col_id_in_grp, v_d_trd_col_id_in_mfma);

		T_Var v_d_row_base_addr = newVgpr("s_b_row_base_addr");
		T_Var v_d_col_offset_addr = newVgpr("s_b_col_offset_addr");
		op3("v_mul_lo_u32", v_tmp1, s_args[argidx_StrD0], v_d_trd_row_idx);
		op3("v_lshlrev_b32", v_d_row_base_addr, log2Int(elem_sz), v_tmp1);
		op3("v_lshlrev_b32", v_d_col_offset_addr, log2Int(GPR_SZ), v_d_trd_col_idx);

		d_glb_step1 = mfma_lane_shape_m * elem_sz * mfma_lane_num_per_tile;
		if (k_param.DataType == E_DataType::Fp16)	d_glb_step1 = 16;
		if (k_param.DataType == E_DataType::Bf16)	d_glb_step1 = 16;
		d_glb_step2 = mfma_m * elem_sz;
		T_Var v_d_glb_step3 = newVgpr("d_glb_step3");
		op3("v_lshlrev_b32", v_tmp1, log2Int(mfma_n), s_args[argidx_StrD0]);
		op3("v_lshlrev_b32", v_d_glb_step3, log2Int(elem_sz), v_tmp1);
		
		v_d_store_offset = newVgpr("d_store_offset", mfma_pttn1_per_wv);
		op3("v_add_u32", v_d_store_offset, v_d_row_base_addr, v_d_col_offset_addr);
		for (uint32_t i = 0; i < mfma_pttn1_per_wv - 1; i++)
			op3("v_add_u32", v_d_store_offset + (i + 1), v_d_glb_step3, v_d_store_offset + i);		

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
	
	// =======================================================================
	void fetch_loop()
	{
		if (write_lds_waitcnt < 0)
			write_lds_waitcnt = (a_fetch_times + b_fetch_times) * (k_param.lds_buffer_num - 2);
		if ((en_direct_glb_to_lds == true) && (write_lds_waitcnt > 63))
			write_lds_waitcnt = 63;
		if ((en_direct_glb_to_lds == false) && (write_lds_waitcnt > 15))
			write_lds_waitcnt = 15;

		if (en_direct_glb_to_lds)
		{
			s_lds_write_cnt = newSgpr("lds_write_cnt");
			s_lds_write_cnt_bck = newSgpr("lds_write_cnt_bck");
			op2("s_mov_b32", s_lds_write_cnt_bck, 0);
		}
		else
		{
			v_lds_write_cnt = newVgpr("lds_write_cnt");
			v_lds_write_cnt_bck = newVgpr("lds_write_cnt_bck");
			op2("v_mov_b32", v_lds_write_cnt_bck, 0);
		}

		//	return;
		// =======================================================================
		uint32_t ext_loop_cnt = k_param.lds_buffer_num - 2;
		if (k_param.K / k_param.loop_unroll < ext_loop_cnt)
			ext_loop_cnt = k_param.K / k_param.loop_unroll;

		T_Var s_loop_cnt = newSgpr("LOOP_CNT");
		T_Var l_end_fetch_loop = newLaber("END_FETCH_LOOP");
		op3("s_lshr_b32", s_loop_cnt, k_param.K, log2Int(k_param.loop_unroll));
		if (k_param.lds_buffer_num > 2)
		{
			op2("s_cmp_le_u32", s_loop_cnt, k_param.lds_buffer_num - 2);
			op1("s_cbranch_scc1", l_end_fetch_loop);
			op3("s_sub_u32", s_loop_cnt, s_loop_cnt, k_param.lds_buffer_num - 2);
		}

		// =======================================================================
		for (int i = 0; i < ext_loop_cnt; i++)
		{
			if (en_direct_glb_to_lds)
			{
				fetch_glb_to_lds_loop();
			}
			else
			{
				fetch_glb_to_gpr_loop();
			}
		}

		// =======================================================================
		T_Var s_fetch_loop_cnt = f_s_loop(s_loop_cnt, "FETCH_LOOP");
		{
			if (en_direct_glb_to_lds)
			{
				fetch_glb_to_lds_loop();
				s_wait_vmcnt(write_lds_waitcnt);
				op0("s_barrier");
			}
			else
			{
				fetch_glb_to_gpr_loop();
				s_wait_lgkmcnt(write_lds_waitcnt);
				op0("s_barrier");
			}
		}
		f_e_loop(s_fetch_loop_cnt, "FETCH_LOOP");
		wrLaber(l_end_fetch_loop);
		delVar(s_loop_cnt);

		// =======================================================================
		for (int i = 0; i < ext_loop_cnt; i++)
		{
			uint32_t waitcnt = (ext_loop_cnt - 1 - i) * write_lds_waitcnt;

			if (en_direct_glb_to_lds)
			{
				if (waitcnt > 63)
					waitcnt = 63;
				s_wait_vmcnt(waitcnt);
			}
			else
			{
				if (waitcnt > 15)
					waitcnt = 15;
				s_wait_lgkmcnt(waitcnt);
			}
			op0("s_barrier");
		}
	}

	void switch_lds_write()
	{
		if (en_direct_glb_to_lds)
		{
			if (k_param.lds_buffer_num == 2)
			{
				op3("s_xor_b32", s_a_lds_write, s_a_lds_write_2, s_a_lds_write);
				op3("s_xor_b32", s_b_lds_write, s_b_lds_write_2, s_b_lds_write);
				op3("s_xor_b32", s_a_lds_write_1, s_a_lds_write_2, s_a_lds_write);
				op3("s_xor_b32", s_b_lds_write_1, s_b_lds_write_2, s_b_lds_write);
			}
			else if (k_param.lds_buffer_num >= 3)
			{
				op3("s_add_u32", s_lds_write_cnt_bck, s_lds_write_cnt_bck, 1);
				op2("s_cmp_eq_i32", s_lds_write_cnt_bck, k_param.lds_buffer_num);
				op3("s_cselect_b32", s_lds_write_cnt, 0, s_lds_write_cnt_bck);
				op2("s_mov_b32", s_lds_write_cnt_bck, s_lds_write_cnt);
				op3("s_mul_i32", s_a_lds_write_1, s_a_lds_write_2, s_lds_write_cnt);
				op3("s_add_u32", s_a_lds_write, s_a_lds_write_0, s_a_lds_write_1);
				op3("s_mul_i32", s_b_lds_write_1, s_b_lds_write_2, s_lds_write_cnt);
				op3("s_add_u32", s_b_lds_write, s_b_lds_write_0, s_b_lds_write_1);
			}
		}
		else
		{
			if (k_param.lds_buffer_num == 2)
			{
				op3("v_xor_b32", v_a_lds_write, v_a_lds_write_2, v_a_lds_write);
				op3("v_xor_b32", v_b_lds_write, v_b_lds_write_2, v_b_lds_write);
				op3("v_xor_b32", v_a_lds_write_1, v_a_lds_write_2, v_a_lds_write);
				op3("v_xor_b32", v_b_lds_write_1, v_b_lds_write_2, v_b_lds_write);
			}
			else if (k_param.lds_buffer_num >= 3)
			{
				op3("v_add_u32", v_lds_write_cnt_bck, v_lds_write_cnt_bck, 1);
				op3("v_cmp_eq_u32", "vcc", v_lds_write_cnt_bck, k_param.lds_buffer_num);
				op4("v_cndmask_b32", v_lds_write_cnt, v_lds_write_cnt_bck, 0, "vcc");
				op2("v_mov_b32", v_lds_write_cnt_bck, v_lds_write_cnt);

				op4("v_mad_u32_u24", v_a_lds_write, v_a_lds_write_2, v_lds_write_cnt, v_a_lds_write_0);
				op4("v_mad_u32_u24", v_b_lds_write, v_b_lds_write_2, v_lds_write_cnt, v_b_lds_write_0);
			}
		}
	}
	void move_to_next_glb_fetch()
	{
		for (uint32_t i = 0; i < a_fetch_times; i++)
			op3("v_add_u32", v_a_fetch_offset + i, k_param.loop_unroll * elem_sz, v_a_fetch_offset + i);
		
		for (uint32_t i = 0; i < b_fetch_times; i++)
			op3("v_add_u32", v_b_fetch_offset + i, k_param.loop_unroll * elem_sz, v_b_fetch_offset + i);
	}
	
	void fetch_glb_to_gpr_loop()
	{
		for (uint32_t i = 0; i < a_fetch_times; i++)
			buffer_load_dword(a_fetch_instr_dw_sz, v_glb_load_a + a_fetch_instr_sz * i, v_a_fetch_offset + i, s_a_dscp, 0, false, true, false, 0);
		for (uint32_t i = 0; i < b_fetch_times; i++)
			buffer_load_dword(b_fetch_instr_dw_sz, v_glb_load_b + b_fetch_instr_sz * i, v_b_fetch_offset + i, s_b_dscp, 0, false, true, false, 0);

		s_wait_vmcnt(0);

		for (uint32_t i = 0; i < a_fetch_times; i++)
			ds_write_dword(lds_wr_instr_dw_sz, v_a_lds_write, v_glb_load_a + a_fetch_instr_sz * i, a_lds_write_step * i);
		for (uint32_t i = 0; i < b_fetch_times; i++)
			ds_write_dword(lds_wr_instr_dw_sz, v_b_lds_write, v_glb_load_b + b_fetch_instr_sz * i, b_lds_write_step * i);

		switch_lds_write();
		move_to_next_glb_fetch();
	}
	void fetch_glb_to_lds_loop()
	{
		op2("s_mov_b32", "m0", s_a_lds_write);
		for (uint32_t i = 0; i < a_fetch_times; i++)
			buffer_load_dword(1, v_tmp1, v_a_fetch_offset + i, s_a_dscp, 0, false, true, true, a_lds_sz_per_wv_per_time * i);

		op2("s_mov_b32", "m0", s_b_lds_write);
		for (uint32_t i = 0; i < b_fetch_times; i++)
			buffer_load_dword(1, v_tmp1, v_b_fetch_offset + i, s_b_dscp, 0, false, true, true, b_lds_sz_per_wv_per_time * i);

		switch_lds_write();
		move_to_next_glb_fetch();
	}

	// =======================================================================
	void math_loop()
	{
		if (read_lds_waitcnt < 0)
			read_lds_waitcnt = mfma_pttn0_per_wv + mfma_pttn1_per_wv;

		v_mfma_a_ping = newVgpr("mfma_a", mfma_agpr_per_mfma * mfma_pttn0_per_wv);
		v_mfma_b_ping = newVgpr("mfma_b", mfma_bgpr_per_mfma * mfma_pttn1_per_wv);
		v_mfma_a_pang = newVgpr("mfma_a", mfma_agpr_per_mfma * mfma_pttn0_per_wv);
		v_mfma_b_pang = newVgpr("mfma_b", mfma_bgpr_per_mfma * mfma_pttn1_per_wv);

		s_lds_write_cnt = newSgpr("lds_write_cnt");
		s_lds_write_cnt_bck = newSgpr("lds_write_cnt_bck");
		op2("s_mov_b32", s_lds_write_cnt_bck, 0);

		op1("s_setprio", 1);
		if (k_param.lds_buffer_num > 2)
		{
			v_lds_read_cnt = newVgpr("lds_read_cnt");
			v_lds_read_cnt_bck = newVgpr("lds_read_cnt");
			op2("v_mov_b32", v_lds_read_cnt_bck, 0);
		}

		//	return;
		// =======================================================================
		op0("s_barrier");
		
		read_lds_to_mfma_ping(0);

		// =======================================================================
		T_Var l_end_math_loop = newLaber("END_MATH_LOOP"); 
		op3("s_lshr_b32", s_tmp1, k_param.K, log2Int(k_param.loop_unroll));
		op2("s_cmp_le_u32", s_tmp1, 1);
		op1("s_cbranch_scc1", l_end_math_loop);
		op3("s_sub_u32", s_tmp1, s_tmp1, 1);

		T_Var s_gemm_loop_cnt = f_s_loop(s_tmp1, "MATH_LOOP");
		{
			math_lds_loop();
		}
		f_e_loop(s_gemm_loop_cnt, "MATH_LOOP");
		wrLaber(l_end_math_loop);

		// =======================================================================
		math_lds_exit_loop();
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
		else if (k_param.lds_buffer_num >= 3)
		{
			op3("v_add_u32", v_lds_read_cnt_bck, v_lds_read_cnt_bck, 1);
			op3("v_cmp_eq_u32", "vcc", v_lds_read_cnt_bck, k_param.lds_buffer_num);
			op4("v_cndmask_b32", v_lds_read_cnt, v_lds_read_cnt_bck, 0, "vcc");
			op2("v_mov_b32", v_lds_read_cnt_bck, v_lds_read_cnt);
			op4("v_mad_u32_u24", v_a_lds_read, v_a_lds_read_2, v_lds_read_cnt, v_a_lds_read_0);
			op4("v_mad_u32_u24", v_b_lds_read, v_b_lds_read_2, v_lds_read_cnt, v_b_lds_read_0);
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
			uint32_t mfma_idx = (m1 + n1 * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

			op4(mfma_inst,
				(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma,
				(v_mfma_a_pang + a_idx_offset) ^ mfma_agpr_per_mfma,
				(v_mfma_b_pang + b_idx_offset) ^ mfma_bgpr_per_mfma,
				(acc_mfma_d + acc_idx_offset) ^ mfma_dgpr_per_mfma);

			for (uint32_t i = 0; i < mfma_dgpr_per_mfma; i++)
			{
				op2("v_accvgpr_read", v_rslt_d + i, (acc_mfma_d + (mfma_idx + i)) ^ 1);

				if (k_param.DataType == E_DataType::Fp32)
				{
					op1("s_nop", 4);
					op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);

					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(d_store_instr_dw_sz,
							v_rslt_d + 4 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				if (k_param.DataType == E_DataType::Fp16)
				{
					op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);
					op2("v_cvt_f16_f32", v_rslt_d + i, v_rslt_d + i);

					if ((i + 1) % 2 == 0)
					{
						op3("v_pack_b32_f16", v_rslt_d + (i / 2), v_rslt_d + (i - 1), v_rslt_d + i);
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(d_store_instr_dw_sz,
							v_rslt_d + 2 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				if (k_param.DataType == E_DataType::Bf16)
				{
					op1("s_nop", 4);
					op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);

					if ((i + 1) % 2 == 0)
					{
						op3("v_lshrrev_b32", v_rslt_d + (i - 1), 16, v_rslt_d + (i - 1));
						op4("v_and_or_b32", v_rslt_d + (i / 2), v_rslt_d + i, v_bf16_msk, v_rslt_d + (i - 1));
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(d_store_instr_dw_sz,
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
			uint32_t mfma_idx = (m1 + n1 * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

			if (loop_num == 1)
			{
				op1("s_nop", 32);
			}

			for (uint32_t i = 0; i < mfma_dgpr_per_mfma; i++)
			{
				op2("v_accvgpr_read", v_rslt_d + i, (acc_mfma_d + (mfma_idx + i)) ^ 1);

				if (k_param.DataType == E_DataType::Fp32)
				{
					op1("s_nop", 4);
					op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);

					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(d_store_instr_dw_sz,
							v_rslt_d + 4 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				if (k_param.DataType == E_DataType::Fp16)
				{
					op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);
					op2("v_cvt_f16_f32", v_rslt_d + i, v_rslt_d + i);

					if ((i + 1) % 2 == 0)
					{
						op3("v_pack_b32_f16", v_rslt_d + (i / 2), v_rslt_d + (i - 1), v_rslt_d + i);
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(d_store_instr_dw_sz,
							v_rslt_d + 2 * (i / 4),
							v_d_store_offset + n1,
							s_d_dscp, 0, false, true,
							instr_offset);
					}
				}
				if (k_param.DataType == E_DataType::Bf16)
				{
					op1("s_nop", 4);
					op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);

					if ((i + 1) % 2 == 0)
					{
						op3("v_lshrrev_b32", v_rslt_d + (i - 1), 16, v_rslt_d + (i - 1));
						op4("v_and_or_b32", v_rslt_d + (i / 2), v_rslt_d + i, v_bf16_msk, v_rslt_d + (i - 1));
					}
					if ((i + 1) % 4 == 0)
					{
						uint32_t instr_offset = m1 * d_glb_step2 + d_glb_step1 * (i / 4);

						buffer_store_dword(d_store_instr_dw_sz,
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
		read_lds_to_mfma_pang(1);

		for (uint32_t k = 0; k < k_param.loop_unroll / mfma_k / 2 - 1; k++)
		{
			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_ping();

			read_lds_to_mfma_ping(2 * (k + 1) + 0);

			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_pang();

			read_lds_to_mfma_pang(2 * (k + 1) + 1);
		}

		s_wait_lgkmcnt(read_lds_waitcnt);
		mfma_mfma_ping_with_lds_switch();

		s_wait_lgkmcnt(0);
		op0("s_barrier");
		mfma_mfma_pang_with_read_lds();
	}
	void math_lds_exit_loop()
	{
		read_lds_to_mfma_pang(1);

		for (uint32_t k = 0; k < k_param.loop_unroll / mfma_k / 2 - 1; k++)
		{
			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_ping();

			read_lds_to_mfma_ping(2 * (k + 1) + 0);

			s_wait_lgkmcnt(read_lds_waitcnt);
			mfma_mfma_pang();

			read_lds_to_mfma_pang(2 * (k + 1) + 1);
		}

		s_wait_lgkmcnt(read_lds_waitcnt);
		mfma_mfma_ping();

		s_wait_lgkmcnt(0);

		mfma_mfma_pang_with_store();
		//mfma_mfma_pang();
		//store_result();
	}

	// =======================================================================
	void fetch_c()
	{
		// -----------------------------------------------------------------------
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t mfma_base_c_idx = (m + n * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

				for (uint32_t t = 0; t < mfma_tile_num_per_mfma; t++)
				{
					uint32_t c_idx = mfma_lane_shape_m * t + mfma_base_c_idx;
					uint32_t instr_offset = m * c_glb_step2 + c_glb_step1 * t;

					buffer_load_dword(c_fetch_instr_dw_sz,
						v_glb_load_c + c_idx,
						v_c_fetch_offset + n,
						s_c_dscp, 0, false, true, false, instr_offset);
				}
			}
		}

		// -----------------------------------------------------------------------
		if (k_param.DataType == E_DataType::Fp16)
		{
			op2("v_cvt_f32_f16", v_tmp1, s_args[argidx_Alpha]);
			op2("v_cvt_f32_f16", v_tmp2, s_args[argidx_Beta]);
			if (k_param.enTensileLayout == true)
			{
				op2("v_readfirstlane_b32", s_args[argidx_Alpha], v_tmp1);
				op2("v_readfirstlane_b32", s_args[argidx_Beta], v_tmp2);
			}
		}

		// -----------------------------------------------------------------------
		for (uint32_t n = 0; n < mfma_pttn1_per_wv; n++)
		{
			for (uint32_t m = 0; m < mfma_pttn0_per_wv; m++)
			{
				uint32_t mfma_base_c_idx = (m + n * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

				uint32_t waitcnt = (mfma_pttn0_per_wv - 1 - m) * (mfma_pttn1_per_wv - 1 - n) * mfma_tile_num_per_mfma;
				s_wait_vmcnt(waitcnt);

				for (uint32_t t = 0; t < mfma_tile_num_per_mfma; t++)
				{
					uint32_t c_idx = mfma_lane_shape_m * t + mfma_base_c_idx;

					for (uint32_t i = 0; i < c_fetch_instr_dw_sz; i++)
					{
						if (k_param.DataType == E_DataType::Fp32)
						{
							op3("v_mul_f32", v_glb_load_c + c_idx + i, s_args[argidx_Beta], v_glb_load_c + c_idx + i);
						}
						if (k_param.DataType == E_DataType::Fp16)
						{
							int j = c_fetch_instr_dw_sz - 1 - i;
							int idx_cvt = c_idx + j;
							int idx_lw = c_idx + j * 2;
							int idx_hi = c_idx + j * 2 + 1;
							op2("v_cvt_f32_f16", v_tmp1, v_glb_load_c + idx_cvt);
							op3("v_lshrrev_b32", v_glb_load_c + idx_cvt, 16, v_glb_load_c + idx_cvt);
							op2("v_cvt_f32_f16", v_glb_load_c + idx_hi, v_glb_load_c + c_idx + j);

							op3("v_mul_f32", v_glb_load_c + idx_lw, s_args[argidx_Beta], v_tmp1);
							op3("v_mul_f32", v_glb_load_c + idx_hi, s_args[argidx_Beta], v_glb_load_c + idx_hi);
						}
						if (k_param.DataType == E_DataType::Bf16)
						{
							op3("v_mul_f32", v_glb_load_c + c_idx + i, s_args[argidx_Beta], v_glb_load_c + c_idx + i);
						}
					}
				}
			}
		}
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
				uint32_t mfma_idx = (m + n * mfma_pttn0_per_wv) * mfma_dgpr_per_mfma;

				for (uint32_t i = 0; i < mfma_dgpr_per_mfma; i++)
				{
					op2("v_accvgpr_read", v_rslt_d + i, (acc_mfma_d + (mfma_idx + i)) ^ 1);

					if (k_param.DataType == E_DataType::Fp32)
					{
						op1("s_nop", 4);
						op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);

						if ((i + 1) % 4 == 0)
						{
							uint32_t instr_offset = m * d_glb_step2 + d_glb_step1 * (i / 4);

							buffer_store_dword(d_store_instr_dw_sz,
								v_rslt_d + 4 * (i / 4),
								v_d_store_offset + n,
								s_d_dscp, 0, false, true,
								instr_offset);
						}
					}
					if (k_param.DataType == E_DataType::Fp16)
					{
						op1("s_nop", 4);
						op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);
						op2("v_cvt_f16_f32", v_rslt_d + i, v_rslt_d + i);

						if ((i + 1) % 2 == 0)
						{
							op3("v_pack_b32_f16", v_rslt_d + (i / 2), v_rslt_d + (i - 1), v_rslt_d + i);
						}
						if ((i + 1) % 4 == 0)
						{
							uint32_t instr_offset = m * d_glb_step2 + d_glb_step1 * (i / 4);

							buffer_store_dword(d_store_instr_dw_sz,
								v_rslt_d + 2 * (i / 4),
								v_d_store_offset + n,
								s_d_dscp, 0, false, true,
								instr_offset);
						}
					}
					if (k_param.DataType == E_DataType::Bf16)
					{
						op1("s_nop", 4);
						op4("v_fma_f32", v_rslt_d + i, s_args[argidx_Alpha], v_rslt_d + i, v_glb_load_c + mfma_idx + i);

						if ((i + 1) % 2 == 0)
						{
							op3("v_lshrrev_b32", v_rslt_d + (i - 1), 16, v_rslt_d + (i - 1));
							op4("v_and_or_b32", v_rslt_d + (i / 2), v_rslt_d + i, v_bf16_msk, v_rslt_d + (i - 1));
						}
						if ((i + 1) % 4 == 0)
						{
							uint32_t instr_offset = m * d_glb_step2 + d_glb_step1 * (i / 4);

							buffer_store_dword(d_store_instr_dw_sz,
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
	void free_fetch_gprs()
	{
		delVar(s_a_dscp);
		delVar(v_a_fetch_offset);

		delVar(s_b_dscp);
		delVar(v_b_fetch_offset);

		if (en_direct_glb_to_lds)
		{
			delVar(s_a_lds_write);
			delVar(s_a_lds_write_0);
			delVar(s_a_lds_write_1);
			delVar(s_a_lds_write_2);

			delVar(s_b_lds_write);
			delVar(s_b_lds_write_0);
			delVar(s_b_lds_write_1);
			delVar(s_b_lds_write_2);

			delVar(s_lds_write_cnt);
			delVar(s_lds_write_cnt_bck);
		}
		else
		{
			delVar(v_glb_load_a);
			delVar(v_glb_load_b);

			delVar(v_a_lds_write);
			delVar(v_a_lds_write_0);
			delVar(v_a_lds_write_1);
			delVar(v_a_lds_write_2);

			delVar(v_b_lds_write);
			delVar(v_b_lds_write_0);
			delVar(v_b_lds_write_1);
			delVar(v_b_lds_write_2);

			delVar(v_lds_write_cnt);
			delVar(v_lds_write_cnt_bck);
		}
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

		delVar(s_c_dscp);
		delVar(v_c_fetch_offset);

		delVar(s_d_dscp);
		delVar(v_d_store_offset);

		delVar(acc_mfma_d);
		delVar(v_rslt_d);

		if (k_param.lds_buffer_num > 2)
		{
			delVar(v_lds_read_cnt);
			delVar(v_lds_read_cnt_bck);
		}
	}
	void free_gprs()
	{
		delVar(v_tmp1); delVar(v_tmp2);
		delVar(s_tmp1);	delVar(s_tmp2);

		delVar(v_tid_in_wave);
		delVar(v_wvid_in_grp);
		delVar(s_wvid_in_grp);
		delVar(s_bid_in_glb);
		delVar(s_wvid_in_glb);
	}
};

