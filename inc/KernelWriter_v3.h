#pragma once

#include "IsaGenerater.h"

namespace feifei
{
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
		ChkErr(writeProgramFrameWork());
		en_dbg_print = false;

		clearString();
		ChkErr(writeSignature());
		ChkErr(writeCodeObj());
		ChkErr(writeMetadata());
		ChkErr(writeProgramFrameWork());

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

	// -----------------------------------------------------------------------
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

	// =======================================================================
	E_ReturnState writeSignature()
	{
		setTable(0);
		//wrLine(".amdgcn_target \"amdgcn-amd-amdhsa--gfx906+sram-ecc\"");
		wrLine(".amdgcn_target \"amdgcn-amd-amdhsa--gfx908+sram-ecc\"");
		wrLine(".text");
		wrLine(".protected " + kernelName);
		wrLine(".globl " + kernelName);
		wrLine(".p2align 8");
		wrLine(".type " + kernelName + ",@function");
		wrLine(".section .rodata,#alloc");
		wrLine(".p2align 6");

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState writeCodeObj()
	{
		setTable(0);
		wrLine(".amdhsa_kernel " + kernelName);
		wrLine("  .amdhsa_user_sgpr_kernarg_segment_ptr 1");
		wrLine("  .amdhsa_next_free_vgpr " + d2s(vgprCountMax));
		wrLine("  .amdhsa_next_free_sgpr " + d2s(sgprCountMax + 6));
		wrLine("  .amdhsa_group_segment_fixed_size " + d2s(ldsByteCount));
		wrLine("  .amdhsa_private_segment_fixed_size 0");
		wrLine("  .amdhsa_system_sgpr_workgroup_id_x 1");
		wrLine("  .amdhsa_system_sgpr_workgroup_id_y 1");
		wrLine("  .amdhsa_system_sgpr_workgroup_id_z 1");
		wrLine("  .amdhsa_system_vgpr_workitem_id 2");
		wrLine(".end_amdhsa_kernel");
		wrLine(".text");
		wrLine("");

		if (sgprCountMax >= MAX_SGPR_COUNT)	return E_ReturnState::RTN_ERR;
		if (vgprCountMax >= MAX_VGPR_COUNT)	return E_ReturnState::RTN_ERR;
		if (ldsByteCount > MAX_LDS_SIZE)	return E_ReturnState::RTN_ERR;

		LOG("sgpr usage = %d.", sgprCountMax);
		LOG("vgpr usage = %d.", vgprCountMax);
		LOG("lds  usage = %.3f(KB).", ldsByteCount / 1024.0);

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
		wrLine(".amdgpu_metadata");
		wrLine("---");
		wrLine("amdhsa.version: [ 1, 0 ]");
		wrLine("amdhsa.kernels:");
		wrLine("  - .name: " + kernelName);
		wrLine("    .symbol: '" + kernelName + ".kd'");
		wrLine("    .language: OpenCL C");
		wrLine("    .language_version: [ 2, 0 ]");
		wrLine("    .group_segment_fixed_size: " + d2s(ldsByteCount));
		wrLine("    .kernarg_segment_align: 8");
		wrLine("    .kernarg_segment_size: " + d2s(argSize));
		wrLine("    .max_flat_workgroup_size: " + d2s(group_sz.x * group_sz.y * group_sz.z));
		wrLine("    .private_segment_fixed_size: 0");
		wrLine("    .sgpr_count: " + d2s(sgprCountMax + 6));
		wrLine("    .sgpr_spill_count: 0");
		wrLine("    .vgpr_count: " + d2s(vgprCountMax));
		wrLine("    .vgpr_spill_count: 0");
		wrLine("    .wavefront_size: 64");
		wrLine("    .args:");
		uint32_t argOffset = 0;
		for (T_KernelArg arg : kernelArgs)
		{
			wrLine("      - .name: " + arg.name);
			wrLine("        .size: " + d2s(arg.argSz));
			wrLine("        .offset: " + d2s(argOffset)); argOffset += arg.argSz;
			if (arg.argKind == E_ArgKind::Global)	wrLine("        .value_kind: global_buffer");
			if (arg.argKind == E_ArgKind::Value)	wrLine("        .value_kind: by_value");
			if (arg.argSz == 4 && arg.argKind == E_ArgKind::Value)	wrLine("        .value_type: u32");
			if (arg.argSz == 8 && arg.argKind == E_ArgKind::Value)	wrLine("        .value_type: u64");
			if (arg.argSz == 4 && arg.argKind == E_ArgKind::Global)	wrLine("        .value_type: f32");
			if (arg.argSz == 8 && arg.argKind == E_ArgKind::Global)	wrLine("        .value_type: f32");
			if (arg.argKind == E_ArgKind::Global)	wrLine("        .address_space: generic");
		}

		wrLine("...");
		wrLine(".end_amdgpu_metadata");

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState writeProgramFrameWork()
	{
		setTable(0);
		clrVar();	dbg_data_cnt = 0;

		l_start_prog = newLaber("START_PROG");
		l_end_prg = newLaber("END_PROG");

		s_argsAddr = newSgpr("s_argsAddr", 2);
		s_bid_x = newSgpr("s_bid_x");
		s_bid_y = newSgpr("s_bid_y");
		s_bid_z = newSgpr("s_bid_z");

		v_tid_x = newVgpr("v_tid_x");
		v_tid_y = newVgpr("v_tid_y");
		v_tid_z = newVgpr("v_tid_z");

		wrLine(kernelName + ":");
		wrLaber(l_start_prog);
		indent();
		ChkErr(writeProgramDetail());
		setTable(0);
		wrLaber(l_end_prg);
		indent();
		wrLine("s_endpgm\n");

		return E_ReturnState::SUCCESS;
	}
	virtual E_ReturnState checkKernelParameters() 
	{
		return E_ReturnState::SUCCESS; 
	};
	virtual E_ReturnState writeProgramDetail() = 0;

	// =======================================================================
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
};
}
