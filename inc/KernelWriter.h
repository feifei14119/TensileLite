/************************************************************************/
/* 这里定义的是依赖于问题配置的相关生成kernel的函数							*/
/* 比如group size，传入参数列表等等										*/
/* 因此只需要include ProblemControl.h									*/
/************************************************************************/
#pragma once

#include "IsaGenerater.h"

namespace feifei
{
#define		KERNEL_DEBUG	(0)

typedef enum class ArgKindEnum
{
	Global = 1,
	Value = 2
}E_ArgKind;
typedef struct KernelArgType
{
	std::string name;
	size_t argSz;
	E_ArgKind argKind;
	bool isConst;
}T_KernelArg;

class KernelWriter : public IsaGenerater
{
public:
	KernelWriter() : IsaGenerater()
	{
		kernelArgs.clear();
		ldsByteCount = 0;
		kernelDir = (GpuRuntime::GetInstance())->KernelTempDir();
		ensure_dir(kernelDir.c_str());
	}

public:
	// TODO: where to clear LDS used byte???
	E_ReturnState GenKernelString()
	{
		ChkErr(checkKernelParameters());

		clearString();
		en_dbg_print = true;
		ChkErr(writeContent());
		en_dbg_print = false;

		clearString();
		ChkErr(writeSignature());
		ChkErr(writeContent());
		ChkErr(writeMetadata());

		if (createStatus != E_ReturnState::SUCCESS)
			return E_ReturnState::RTN_ERR;

		return E_ReturnState::SUCCESS;
	}
	void SaveKernelString2File()
	{
		ensure_dir(kernelDir.c_str());
		kernelFile = kernelDir + DIR_SPT + kernelName + "_AutoGen.s";
		dump2_txt_file(kernelFile, kernelString);
	}

	void KernelName(std::string name) { kernelName = name; }
	void KernelDirectory(std::string dir) { kernelDir = dir; ensure_dir(kernelDir.c_str()); }
	std::string KernelDirectory() { return kernelDir; }
	std::string KernelFile() { return kernelFile; }
	std::string KernelString() { return kernelString; }
	std::string KernelName() { return kernelName; }
	dim GroupSize() { return group_sz; }
	dim GlobalSize() { return global_sz; }

	E_ReturnState SetArg(std::string name = "", size_t argSz = 8, E_ArgKind kind = E_ArgKind::Global, bool isConst = false)
	{
		T_KernelArg arg;

		arg.argKind = kind;
		arg.argSz = argSz;
		arg.name = name;
		arg.isConst = isConst;
		kernelArgs.push_back(arg);

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState SetDispatch(T_Dispatch disp)
	{
		global_sz = disp.global_size;
		group_sz = disp.group_size;
		group_num = global_sz / group_sz;
		dispatch = disp;
	}
	T_Dispatch GetDispatch() { return dispatch; }

protected:
	std::string kernelName;
	std::string kernelDir;
	std::string kernelFile;
	dim group_sz;
	dim group_num;
	dim global_sz;
	T_Dispatch dispatch;
	std::vector<T_KernelArg> kernelArgs;

	// =======================================================================
	// 默认寄存器和段名
	// =======================================================================
	//T_Var s_privateSeg;
	T_Var s_argsAddr;
	std::vector<T_Var> s_args;
	T_Var s_bid_x;
	T_Var s_bid_y;
	T_Var s_bid_z;

	T_Var v_tid_x;
	T_Var v_tid_y;
	T_Var v_tid_z;

	T_Var l_start_prog;
	T_Var l_end_prg;

	uint32_t dbg_data_cnt = 0;
	uint32_t dbg_buff_len = 2; // dword
	bool en_dbg_print = false;
	
	/************************************************************************/
	/* kernel文件生成函数                                                    */
	/************************************************************************/
	E_ReturnState writeSignature()
	{
		setTable(0);
		wrLine(".hsa_code_object_version 2, 1");
		if (IsaArch == E_IsaArch::Gfx900)
		{
			wrLine(".hsa_code_object_isa 9, 0, 0, \"AMD\", \"AMDGPU\"");
		}
		else if (IsaArch == E_IsaArch::Gfx803)
		{
			wrLine(".hsa_code_object_isa 8, 0, 3, \"AMD\", \"AMDGPU\"");
		}
		else if (IsaArch == E_IsaArch::Gfx906)
		{
			wrLine(".hsa_code_object_isa 9, 0, 6, \"AMD\", \"AMDGPU\"");
		}
		else if (IsaArch == E_IsaArch::Gfx908)
		{
			wrLine(".hsa_code_object_isa 9, 0, 8, \"AMD\", \"AMDGPU\"");			
		}
		wrLine("");
		wrLine(".text");
		wrLine(".globl " + kernelName);
		wrLine(".p2align 8");
		wrLine(".type " + kernelName + ",@function");
		wrLine(".amdgpu_hsa_kernel " + kernelName);
		wrLine("");

		return E_ReturnState::SUCCESS;
	}
	virtual E_ReturnState checkKernelParameters() { return E_ReturnState::SUCCESS; };
	E_ReturnState writeContent()
	{
		setTable(0);
		wrLine(kernelName + ":");
		ChkErr(writeCodeObj());
		clrVar();	dbg_data_cnt = 0;
		ChkErr(initialDefaultGprs());
		ChkErr(writeProgramFrameWork());

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState writeMetadata()
	{
		int argSize = 0;
		for (T_KernelArg arg : kernelArgs)
		{
			argSize += arg.argSz;
		}

		setTable(0);
		wrLine(".amd_amdgpu_hsa_metadata");
		wrLine("{ Version: [1, 0],");
		wrLine("  Kernels :");
		wrLine("    - { Name: " + kernelName + ",");
		wrLine("        SymbolName: " + kernelName + ",");
		if(GpuRuntime::GpuRtType() == E_GpuRt::HIP)
			wrLine("        Language: HCC, LanguageVersion: [ 1, 2 ],");
		else if (GpuRuntime::GpuRtType() == E_GpuRt::OCL)
			wrLine("        Language: OpenCL C, LanguageVersion: [ 1, 2 ],");

		wrLine("        Attrs: { ReqdWorkGroupSize: [ " + d2s(group_sz.x) + ", " + d2s(group_sz.y) + ", " + d2s(group_sz.z) + " ] }");
		wrLine("        CodeProps: { KernargSegmentSize: " + d2s(argSize) + ", GroupSegmentFixedSize : 0, PrivateSegmentFixedSize : 0, KernargSegmentAlign : 8, WavefrontSize : 64, MaxFlatWorkGroupSize : " + d2s(group_sz.x * group_sz.y) + " }");
		
		wrLine("        Args:");
		for (T_KernelArg arg : kernelArgs)
		{
			std::string strtmp = "        - {";
			strtmp += " Name: " + arg.name + ",";
			strtmp += " Size: " + d2s(arg.argSz) + ",";
			strtmp += " Align: " + d2s(arg.argSz) + ",";
			if (arg.argKind == E_ArgKind::Global)		strtmp += " ValueKind: GlobalBuffer,";
			else if(arg.argKind == E_ArgKind::Value)	strtmp += " ValueKind: ByValue,";
			if(arg.argSz == 8 && arg.argKind == E_ArgKind::Value)
				strtmp += " ValueType: F64, TypeName: 'double',";
			else
				strtmp += " ValueType: F32, TypeName: 'float*',";
			if (arg.isConst == true)					strtmp += " AddrSpaceQual: Global, IsConst: true";
			else										strtmp += " AddrSpaceQual: Global";
			strtmp += " }";

			wrLine(strtmp);
		}

		wrLine("      }");
		wrLine("}");
		wrLine(".end_amd_amdgpu_hsa_metadata");
		wrLine("");

		return E_ReturnState::SUCCESS;
	}

	/************************************************************************/
	/* kernel 函数内容生成函数                                                */
	/************************************************************************/
	E_ReturnState initialDefaultGprs()
	{
		//s_privateSeg = newSgpr("s_privateSeg", 4);
		s_argsAddr = newSgpr("s_argsAddr", 2);
		s_bid_x = newSgpr("s_bid_x");
		s_bid_y = newSgpr("s_bid_y");
		s_bid_z = newSgpr("s_bid_z");

		v_tid_x = newVgpr("v_tid_x");
		v_tid_y = newVgpr("v_tid_y");
		v_tid_z = newVgpr("v_tid_z");

		l_start_prog = newLaber("START_PROG");
		l_end_prg = newLaber("END_PROG");

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState writeCodeObj()
	{
		setTable(1);
		wrLine(".amd_kernel_code_t");
		indent();

		wrLine("amd_code_version_major = 1");
		wrLine("amd_code_version_minor = 1");
		wrLine("amd_machine_kind = 1");
		wrLine("amd_machine_version_major = 8");
		wrLine("amd_machine_version_minor = 0");
		wrLine("amd_machine_version_stepping = 3");
		wrLine("kernarg_segment_alignment = 4");
		wrLine("group_segment_alignment = 4");
		wrLine("private_segment_alignment = 4");
		wrLine("wavefront_size = 6");
		wrLine("call_convention = -1");

		//wrLine("enable_sgpr_private_segment_buffer = 1");
		wrLine("enable_sgpr_kernarg_segment_ptr = 1");
		wrLine("enable_sgpr_workgroup_id_x = 1");
		wrLine("enable_sgpr_workgroup_id_y = 1");
		wrLine("enable_sgpr_workgroup_id_z = 1");
		wrLine("enable_vgpr_workitem_id = 2");
		wrLine("is_ptr64 = 1");
		wrLine("float_mode = 192");

		wrLine("granulated_wavefront_sgpr_count = " + d2s((sgprCountMax + 6 + 8 - 1) / 8 - 1));
		wrLine("granulated_workitem_vgpr_count = " + d2s((vgprCountMax + 4 - 1) / 4 - 1));
		wrLine("user_sgpr_count = 2");		// for kernel param list pointer
		wrLine("wavefront_sgpr_count = " + d2s(sgprCountMax + 6));
		wrLine("workitem_vgpr_count = " + d2s(vgprCountMax));
		wrLine("kernarg_segment_byte_size = 44");
		wrLine("workgroup_group_segment_byte_size = " + d2s(ldsByteCount));
		backSpace();
		wrLine(".end_amd_kernel_code_t");
		wrLine("");

		if (sgprCountMax >= MAX_SGPR_COUNT)	return E_ReturnState::RTN_ERR;
		if (vgprCountMax >= MAX_VGPR_COUNT)	return E_ReturnState::RTN_ERR;
		if (ldsByteCount > MAX_LDS_SIZE)	return E_ReturnState::RTN_ERR;

		static bool log_once = false;
		if (log_once == false)
		{
			log_once = true;
		}
		else
		{
			LOG("sgpr usage = %d.", sgprCountMax);
			LOG("vgpr usage = %d.", vgprCountMax);
			LOG("lds  usage = %.3f(KB).", ldsByteCount / 1024.0);
		}

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState writeProgramFrameWork()
	{
		setTable(0);
		wrLaber(l_start_prog);
		indent();
		ChkErr(writeProgramDetail());
		setTable(0);
		wrLaber(l_end_prg);
		indent();
		wrLine("s_endpgm\n");

		return E_ReturnState::SUCCESS;
	}
	virtual E_ReturnState writeProgramDetail() = 0;

	/************************************************************************/
	/* 常用kernel函数														 */
	/************************************************************************/
	void f_load_kernel_args()
	{
		s_args.clear();
		uint32_t bias = 0;
		for (T_KernelArg arg : kernelArgs)
		{
			T_Var s_arg;
			if (arg.argSz == 4)
			{
				s_arg = newSgpr("s_arg");
				s_load_dword(1, s_arg, s_argsAddr, bias);
			}
			else if (arg.argSz == 8)
			{
				s_arg = newSgpr("s_arg", 2, 2);
				s_load_dword(2, s_arg, s_argsAddr, bias);
			}
			bias += arg.argSz;
			s_args.push_back(s_arg);
		}
		
		s_wait_lgkmcnt(0);
	}
	
	void f_debug_data(T_Var s_dbg_base_addr, T_Var dbg_data, bool need_cvt = true)
	{
		if (dbg_data_cnt >= dbg_buff_len)
			return;

		wrComment4("debug dump data " + getVarStr(dbg_data));

		T_Var v_tmp1 = newVgpr("v_tmp1");
		T_Var s_tmp1 = newSgpr("s_tmp1");
		// debug address
	//	T_Var v_dbg_a = newVgpr("dbg_a");
	//	T_Var v_dbg_b = newVgpr("dbg_b");
		uint32_t flat_grp_sz = group_sz.x * group_sz.y;
		T_Var v_dbg_addr = newVgpr("dbg_addr", 2, 2);
		T_Var s_flat_bid = newSgpr("flat_bid");
		T_Var v_flat_tid = newVgpr("flat_tid");
		T_Var v_flat_gid = newVgpr("flat_gid");
		// flat_bid = grid_dim_x * bid_y + bid_x
		op3("s_mul_i32", s_tmp1, s_bid_y, group_num.x);
		op3("s_add_u32", s_flat_bid, s_tmp1, s_bid_x);
		// flat_tid = grid_dim_x * tid_y + tid_x
		op3("v_mul_u32_u24", v_tmp1, group_sz.x, v_tid_y);
		op3("v_add_u32", v_flat_tid, v_tmp1, v_tid_x);
		// flat_gid = flat_grp_sz * flat_bid + flat_tid
		op2("v_mov_b32", v_tmp1, s_flat_bid);
		op3("v_mul_u32_u24", v_tmp1, flat_grp_sz, v_tmp1);
		op3("v_add_u32", v_flat_gid, v_tmp1, v_flat_tid);
		// dbg_addr = dbg_buff + BYTE(flat_gid * dbgNum)
		op3("v_mul_u32_u24", v_tmp1, dbg_buff_len, v_flat_gid);
		op3("v_lshlrev_b32", v_tmp1, 2, v_tmp1);
		op2("v_mov_b32", v_dbg_addr + 1, s_dbg_base_addr + 1);
		op4("v_add_co_u32", v_dbg_addr, "vcc", s_dbg_base_addr, v_tmp1);
		op5("v_addc_co_u32", v_dbg_addr + 1, "vcc", 0, v_dbg_addr + 1, "vcc");

		op2("v_mov_b32", v_tmp1, dbg_data);
		if (need_cvt)
			op2("v_cvt_f32_u32", v_tmp1, v_tmp1);

		flat_store_dword(1, v_dbg_addr, v_tmp1, "off", dbg_data_cnt * GPR_SZ);
		s_wait_cnt0();

		delVar(s_flat_bid);
		delVar(v_flat_tid);
		delVar(v_flat_gid);
		delVar(v_dbg_addr);
		delVar(v_tmp1);
		delVar(s_tmp1);

		dbg_data_cnt++;
	}
	template<typename T> void f_fill_lds(T fill_data)
	{
		T_Var l_start_fill_lds = newLaber("START_FILL_LDS");
		T_Var l_end_fill_lds = newLaber("END_FILL_LDS");

		wrComment4("debug fill LDS " + std::to_string(ldsByteCount / GPR_SZ) + " dwords.");
		wrLaber(l_start_fill_lds);

		T_Var v_tmp1 = newVgpr("v_tmp1");
		T_Var s_tmp1 = newSgpr("s_tmp1");
		T_Var s_flat_bid = newSgpr("flat_bid");
		T_Var v_flat_tid = newVgpr("flat_tid");
		T_Var s_exec_bck = newSgpr("exec_backup", 2, 2);

		// flat_bid = grid_dim_x * bid_y + bid_x
		op3("s_mul_i32", s_tmp1, s_bid_y, group_num.x);
		op3("s_add_u32", s_flat_bid, s_tmp1, s_bid_x);
		// flat_tid = grid_dim_x * tid_y + tid_x
		op3("v_mul_u32_u24", v_tmp1, group_sz.x, v_tid_y);
		op3("v_add_u32", v_flat_tid, v_tmp1, v_tid_x);

		// only first round on cu do debug fill
		op2("s_cmpk_ge_u32", s_flat_bid, CU_NUM); // if(bid >= CU_NUM) scc = 1
		op1("s_cbranch_scc1", l_end_fill_lds); // if(scc == 1) jump to end, dont fill

		// only first wave in group do debug fill
		//op3("v_cmpx_lt_u32", "exec", v_flat_tid, WAVE_SIZE); //if (tid < WAVE_SIZE) exec.tid = 0
		op3("v_cmp_lt_u32", "vcc", v_flat_tid, WAVE_SIZE); //if (tid < WAVE_SIZE) vcc.tid = 1
		op1("s_cbranch_vccz", l_end_fill_lds); // if(vcc == 0) jump to end, dont fill

		// only first thread in wave do debug fill
		op2("s_mov_b64", s_exec_bck ^ 2, "exec");
		op3("v_cmpx_eq_u32", "exec", v_flat_tid, 0); //if (tid < WAVE_SIZE) vcc.tid = 1

		// fill_addr = dbg_buff + BYTE(flat_gid * dbgNum)
		uint32_t fill_dword_per_thread = ldsByteCount / GPR_SZ;
		T_Var v_lds_addr = newVgpr("lds_addr");
		op3("v_mul_u32_u24", v_tmp1, fill_dword_per_thread*GPR_SZ, v_flat_tid);
		op2("v_mov_b32", v_lds_addr, v_tmp1);

		op2("v_mov_b32", v_tmp1, fill_data);

		// 循环填充
		T_Var s_loop_cnt;
		s_loop_cnt = f_s_loop(fill_dword_per_thread, "FILL_LDS_LOOP");
		{
			ds_write_dword(1, v_lds_addr, v_tmp1);
			op3("v_add_u32", v_lds_addr, v_lds_addr, GPR_SZ);
			s_wait_lgkmcnt(0);
		}
		f_e_loop(s_loop_cnt, "FILL_LDS_LOOP");
		op2("s_mov_b64", "exec", s_exec_bck ^ 2);

		delVar(v_tmp1);
		delVar(s_tmp1);
		delVar(s_flat_bid);
		delVar(v_flat_tid);
		delVar(v_lds_addr);
		delVar(s_loop_cnt);
		delVar(s_exec_bck);

		wrLaber(l_end_fill_lds);
		s_wait_cnt0();
		op0("s_barrier");
	}
	void f_dump_lds(T_Var s_dump_base_addr, int32_t grp_idx = -1, bool need_cvt = true)
	{
		T_Var l_start_dump_lds = newLaber("START_DUMP_LDS");
		T_Var l_end_dump_lds = newLaber("END_DUMP_LDS");

		wrComment4("debug dump LDS " + std::to_string(ldsByteCount / GPR_SZ) + " dwords for group " + std::to_string(grp_idx));
		wrLaber(l_start_dump_lds);
		op0("s_barrier");

		T_Var v_tmp1 = newVgpr("v_tmp1");
		T_Var s_tmp1 = newSgpr("s_tmp1");
		T_Var s_flat_bid = newSgpr("flat_bid");
		T_Var v_flat_tid = newVgpr("flat_tid");
		T_Var v_dump_gid = newVgpr("flat_gid");

		// flat_bid = grid_dim_x * bid_y + bid_x
		op3("s_mul_i32", s_tmp1, s_bid_y, group_num.x);
		op3("s_add_u32", s_flat_bid, s_tmp1, s_bid_x);
		// flat_tid = grid_dim_x * tid_y + tid_x
		op3("v_mul_u32_u24", v_tmp1, group_sz.x, v_tid_y);
		op3("v_add_u32", v_flat_tid, v_tmp1, v_tid_x);

		// only first round on cu do dump
		op2("s_cmpk_ge_u32", s_flat_bid, CU_NUM); // if(bid >= CU_NUM) scc = 1
		op1("s_cbranch_scc1", l_end_dump_lds); // if(scc == 1) jump to end, dont dump

		// only first wave in group do dump
		//op3("v_cmpx_lt_u32", "exec", v_flat_tid, WAVE_SIZE); //if (tid < WAVE_SIZE) exec.tid = 0
		op3("v_cmp_lt_u32", "vcc", v_flat_tid, WAVE_SIZE); //if (tid < WAVE_SIZE) vcc.tid = 1
		op1("s_cbranch_vccz", l_end_dump_lds); // if(vcc == 0) jump to end, dont dump

		// only first thread in wave do dump
		T_Var s_exec_bck = newSgpr("exec_backup", 2, 2);
		op2("s_mov_b64", s_exec_bck ^ 2, "exec");
		op3("v_cmpx_eq_u32", "exec", v_flat_tid, 0); //if (tid < WAVE_SIZE) vcc.tid = 1

		// dump persific group
		if (grp_idx >= 0)
		{
			op2("s_cmpk_lg_u32", s_flat_bid, grp_idx);// if(bid != grp_idx) scc = 1
			op1("s_cbranch_scc1", l_end_dump_lds); // if(scc == 1) jump to end, dont dump
			op2("s_mov_b32", s_flat_bid, 0);
		}

		// flat_gid = wave_size * flat_bid + flat_tid
		op2("v_mov_b32", v_tmp1, s_flat_bid);
		op3("v_mul_u32_u24", v_tmp1, WAVE_SIZE, v_tmp1);
		op3("v_add_u32", v_dump_gid, v_tmp1, v_flat_tid);

		// dump_addr = dbg_buff + BYTE(flat_gid * dbgNum)
		uint32_t dump_dword_per_thread = ldsByteCount / GPR_SZ;
		T_Var v_lds_addr = newVgpr("lds_addr");
		T_Var v_dump_addr = newVgpr("dump_addr", 2, 2);
		op3("v_mul_u32_u24", v_tmp1, dump_dword_per_thread*GPR_SZ, v_dump_gid);
		op2("v_mov_b32", v_lds_addr, v_tmp1);

		op3("v_mul_u32_u24", v_tmp1, dump_dword_per_thread*GPR_SZ, v_dump_gid);
		op2("v_mov_b32", v_dump_addr + 1, s_dump_base_addr + 1);
		op4("v_add_co_u32", v_dump_addr, "vcc", s_dump_base_addr, v_tmp1);
		op5("v_addc_co_u32", v_dump_addr + 1, "vcc", 0, v_dump_addr + 1, "vcc");

		// 循环dump
		T_Var s_loop_cnt;
		s_loop_cnt = f_s_loop(dump_dword_per_thread, "DUMP_LDS_LOOP");
		{
			ds_read_dword(1, v_tmp1, v_lds_addr, 0);
			op3("v_add_u32", v_lds_addr, v_lds_addr, GPR_SZ);
			s_wait_lgkmcnt(0);
			if (need_cvt)
				op2("v_cvt_f32_u32", v_tmp1, v_tmp1);

			flat_store_dword(1, v_dump_addr, v_tmp1, "off", 0);
			op4("v_add_co_u32", v_dump_addr, "vcc", v_dump_addr, GPR_SZ);
			op5("v_addc_co_u32", v_dump_addr + 1, "vcc", 0, v_dump_addr + 1, "vcc");
			s_wait_vmcnt(0);
		}
		f_e_loop(s_loop_cnt, "DUMP_LDS_LOOP");

		op2("s_mov_b64", "exec", s_exec_bck ^ 2);

		delVar(v_tmp1);
		delVar(s_tmp1);
		delVar(s_flat_bid);
		delVar(v_flat_tid);
		delVar(v_dump_gid);
		delVar(v_lds_addr);
		delVar(v_dump_addr);
		delVar(s_loop_cnt);
		delVar(s_exec_bck);

		wrLaber(l_end_dump_lds);
		s_wait_cnt0();
		op0("s_barrier");
	}

	/************************************************************************/
	/* 测试函数																*/
	/************************************************************************/
	void print_index(int *index, char* name)
	{
		int grpNumPerCUMax = (int)((group_num.x + CU_NUM - 1) / CU_NUM);
		int grpNumPerCUMin = (int)(group_num.x / CU_NUM);
		int maxGrpCUNum = (int)((group_num.x - grpNumPerCUMin * CU_NUM) / SE_NUM);
		int minGrpCUNum = (int)((CU_NUM - maxGrpCUNum * SE_NUM) / SE_NUM);

		int simuGrpIdx = 0;
		int grpIdxBase;

		printf("\t|---------------------------------------------------------\n");
		printf("\t| index name = %s\n", name);
		printf("\t| work load = [%zd, %zd, %zd] * %zd\n", group_sz.x, group_sz.y, group_sz.z, group_num.x);
		if(maxGrpCUNum == 0)
			printf("\t| group per cu = (%d gp * %d cu)\n", grpNumPerCUMin, minGrpCUNum);
		else
			printf("\t| group per cu = (%d gp * %d cu) + (%d gp * %d cu)\n", grpNumPerCUMax, maxGrpCUNum, grpNumPerCUMin, minGrpCUNum);
		printf("\t|---------------------------------------------------------\n");
		for (int se = 0; se < SE_NUM; se++)
		{
			printf("SE=%d:", se);
			grpIdxBase = se;

			for (int cu = 0; cu < CU_PER_SE; cu++)
			{
				printf("\t[%02d]: ", cu);
				simuGrpIdx = grpIdxBase;

				while (simuGrpIdx < group_num.x)
				{
					printf("%03d, ", index[simuGrpIdx]);
					simuGrpIdx += CU_NUM;
				}
				printf("\n");
				grpIdxBase += 4;
			}
			printf("\n");
		}
	}
};
}
