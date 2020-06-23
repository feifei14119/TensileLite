/************************************************************************/
/* 这里定义的是依赖于问题配置的相关生成kernel的函数							*/
/* 比如group size，传入参数列表等等										*/
/* 因此只需要include ProblemControl.h									*/
/************************************************************************/
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
};
}