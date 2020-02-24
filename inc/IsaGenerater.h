/************************************************************************/
/* 这里定义的是不依赖于问题配置的相关生成kernel的函数						*/
/* 因此只需要include BasicClass.h										*/
/************************************************************************/
#pragma once
#include "../inc/ff_hip_runtime.h"

namespace feifei
{
#define	PARAM_START_COL		(44)
#define	FLAG_START_COL		(85)
#define	COMMON_START_COL	(109)

#define	c2s(c)				std::string(c)
#define	s2c(s)				s.c_str()
#define	d2s(d)				std::to_string(d)
#define	f2s(f)				std::to_string((float)f)
#define d2c(d)				(char*)(s2c(d2s(d)))
#define	d2hs(d)				std::string("0x0+"+std::to_string(d))
#define	c2hs(c)				std::string("0x0+"+c2s(c))

	typedef enum class OpTypeEnum
	{
		NONE = 0,
		ADD, SUB, INC, DEC,
		OR, XOR, AND,
		SMAX, SMIN, UMAX, UMIN,
		SWAP, CMPSWAP,
		CMPEQ, CMPNE, CMPGT, CMPGE, CMPLT, CMPLE
	} E_OpType;

	typedef enum class TokenEnum
	{
		None, Num, Symble, Other, Assign, Cond, Def,
		EndLine, EndProg, EndExpr, EndSymb, OpenExpr, CloseExpr,
		Add, Sub, Mul, Div, Mod, Inc, Dec,
		Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr,
		KeyType, TypeTrans, Free,
		Sys, Glo, Loc, Id, Char, Else, Enum, If, Return, Sizeof, While, Brak, Start
	} E_Token;
	typedef struct TokenType
	{
		E_Token type;
		char valChar;
		int valInt;
		std::string valStr;
	} T_Token;
	typedef enum class StatEnum
	{
		DEF, DECL, EXPR, IF, RTN
	} E_Stat;

	typedef enum class VarTypeEnum
	{
		Sgpr = 1,
		Vgpr = 2,
		Agpr = 3,
		Imm = 4,
		Laber = 5,
		String = 6,
		Macro = 7,
		Off = 0,
		Idle = -1,
		None = -2
	}E_VarType;
	typedef struct GprType
	{
		int perGprIdx;
		int gprIdx;
		int len;			// 为该变量分配的dword个数
		int align;
		int forceLen;		// 强制使用指定的dword个数
		bool forceLenEn;	// 是否强制使用指定的dword个数
		bool isMinus;
	}t_gpr;
	typedef struct VarType
	{
		std::string name;
		E_VarType type;
		int value;
		t_gpr gpr;

		// 使用偏移i个的dword寄存器
		VarType operator+(int i)
		{
			struct VarType np;
			np.name = this->name + "+" + d2s(i);
			np.type = this->type;
			np.gpr = this->gpr;
			np.value = this->value;
			np.gpr.gprIdx = this->gpr.gprIdx + i;

			return np;
		}
		// 使用偏移i个的dword寄存器
		VarType operator[](int i)
		{
			struct VarType np;
			np.name = this->name + "+" + d2s(i);
			np.type = this->type;
			np.value = this->value;
			np.gpr = this->gpr;
			np.gpr.gprIdx = this->gpr.gprIdx + i;
			np.gpr.len = 1;
			np.gpr.forceLenEn = false;
			np.gpr.forceLen = 1;

			return np;
		}

		// 强制使用i个数的dword寄存器
		VarType operator^(int i)
		{
			this->gpr.forceLen = i;
			this->gpr.forceLenEn = true;

			return *this;
		}
		VarType operator-()
		{
			this->gpr.isMinus = true;
			return *this;
		}
	}T_Var;
	
	class IsaGenerater
	{
#ifdef _WIN32
#undef s_addr
#endif
	public:
		IsaGenerater()
		{
			sgprCount = 0;
			vgprCount = 0;
			ldsByteCount = 0;
			memset(SgprState, 0, sizeof(int)*MAX_SGPR_COUNT);
			memset(VgprState, 0, sizeof(int)*MAX_VGPR_COUNT);
			memset(AgprState, 0, sizeof(int)*MAX_AGPR_COUNT);
			IsaArch = GpuRuntime::GetInstance()->Device()->DeviceInfo().Arch;

			idleVar.type = E_VarType::Idle;
		}
		std::string * GetKernelString()
		{
			return &kernelString;
		}

	protected:
		E_IsaArch IsaArch = E_IsaArch::Gfx900;
		std::string kernelString;

		/************************************************************************/
		/* 寄存器与变量操作                                                       */
		/************************************************************************/
		int SgprState[MAX_SGPR_COUNT];
		int VgprState[MAX_VGPR_COUNT];
		int AgprState[MAX_AGPR_COUNT];
		int sgprCount = 0, sgprCountMax = 0;
		int vgprCount = 0, vgprCountMax = 0;
		int agprCount = 0, agprCountMax = 0;
		int ldsByteCount = 0;
		std::map<std::string, T_Var> OperatorMap;
		T_Var idleVar;

		T_Var newSgpr(std::string name, int len = 1, int align = 1)
		{
			T_Var opt;

			if ((name == "off") || (name == "OFF"))
			{
				opt.name = name;
				opt.type = E_VarType::Off;
				opt.value = 0;
			}
			else
			{
				int idleIdx;
				for (idleIdx = 0; idleIdx < MAX_SGPR_COUNT; idleIdx++)
				{
					if (idleIdx % align != 0)
						continue;

					if (SgprState[idleIdx] != 0)
						continue;

					int idleChk;
					for (idleChk = 0; idleChk < len; idleChk++)
					{
						if (SgprState[idleChk + idleIdx] != 0)
							break;
					}
					if (idleChk != len)
						continue;
					break;
				}
				if (idleIdx == MAX_SGPR_COUNT)
				{
					opt.name = "";
					opt.value = 0;
					opt.type = E_VarType::None;
					return opt;
				}

				for (int i = 0; i < len; i++)
				{
					SgprState[idleIdx + i] = 1;
				}

				opt.name = name;
				opt.value = 0;
				opt.type = E_VarType::Sgpr;
				opt.gpr.perGprIdx = sgprCount;
				opt.gpr.gprIdx = idleIdx;
				opt.gpr.len = len;
				opt.gpr.align = align;
				opt.gpr.forceLen = 1;
				opt.gpr.forceLenEn = false;
				opt.gpr.isMinus = false;

				if (idleIdx + len > sgprCountMax)
					sgprCountMax = idleIdx + len;
			}

			OperatorMap.insert(std::pair<std::string, T_Var>(name, opt));
			return opt;
		}
		T_Var newVgpr(std::string name, int len = 1, int align = 1)
		{
			T_Var opt;

			if ((name == "off") || (name == "OFF"))
			{
				opt.name = name;
				opt.value = 0;
				opt.type = E_VarType::Off;
			}
			else
			{
				int idleIdx;
				for (idleIdx = 0; idleIdx < MAX_VGPR_COUNT; idleIdx++)
				{
					if (idleIdx % align != 0)
						continue;

					if (VgprState[idleIdx] != 0)
						continue;

					int idleChk;
					for (idleChk = 0; idleChk < len; idleChk++)
					{
						if (VgprState[idleChk + idleIdx] != 0)
							break;
					}
					if (idleChk != len)
						continue;
					break;
				}
				if (idleIdx == MAX_VGPR_COUNT)
				{
					opt.name = "";
					opt.value = 0;
					opt.type = E_VarType::None;
					return opt;
				}

				for (int i = 0; i < len; i++)
				{
					VgprState[idleIdx + i] = 1;
				}

				opt.name = name;
				opt.value = 0;
				opt.type = E_VarType::Vgpr;
				opt.gpr.perGprIdx = sgprCount;
				opt.gpr.gprIdx = idleIdx;
				opt.gpr.len = len;
				opt.gpr.align = align;
				opt.gpr.forceLen = 1;
				opt.gpr.forceLenEn = false;
				opt.gpr.isMinus = false;

				if (idleIdx + len > vgprCountMax)
					vgprCountMax = idleIdx + len;
			}

			OperatorMap.insert(std::pair<std::string, T_Var>(name, opt));
			return opt;
		}
		T_Var newAgpr(std::string name, int len = 1, int align = 1)
		{
			T_Var opt;

			if ((name == "off") || (name == "OFF"))
			{
				opt.name = name;
				opt.value = 0;
				opt.type = E_VarType::Off;
			}
			else
			{
				int idleIdx;
				for (idleIdx = 0; idleIdx < MAX_AGPR_COUNT; idleIdx++)
				{
					if (idleIdx % align != 0)
						continue;

					if (AgprState[idleIdx] != 0)
						continue;

					int idleChk;
					for (idleChk = 0; idleChk < len; idleChk++)
					{
						if (AgprState[idleChk + idleIdx] != 0)
							break;
					}
					if (idleChk != len)
						continue;
					break;
				}
				if (idleIdx == MAX_AGPR_COUNT)
				{
					opt.name = "";
					opt.value = 0;
					opt.type = E_VarType::None;
					return opt;
				}

				for (int i = 0; i < len; i++)
				{
					AgprState[idleIdx + i] = 1;
				}

				opt.name = name;
				opt.value = 0;
				opt.type = E_VarType::Agpr;
				opt.gpr.perGprIdx = agprCount;
				opt.gpr.gprIdx = idleIdx;
				opt.gpr.len = len;
				opt.gpr.align = align;
				opt.gpr.forceLen = 1;
				opt.gpr.forceLenEn = false;
				opt.gpr.isMinus = false;

				if (idleIdx + len > agprCountMax)
					agprCountMax = idleIdx + len;
			}

			OperatorMap.insert(std::pair<std::string, T_Var>(name, opt));
			return opt;
		}
		T_Var newImm(std::string name, int val = 0)
		{
			T_Var opt;

			opt.name = name;
			opt.type = E_VarType::Imm;
			opt.value = val;
			opt.gpr.len = 0;

			OperatorMap.insert(std::pair<std::string, T_Var>(name, opt));
			return opt;
		}
		T_Var newLaber(std::string laber)
		{
			T_Var opt;
			opt.name = laber;
			opt.type = E_VarType::Laber;
			OperatorMap.insert(std::pair<std::string, T_Var>(laber, opt));
			return opt;
		}
		void delVar(T_Var var)
		{
			if (var.type == E_VarType::Sgpr)
			{
				int gprIdx = var.gpr.gprIdx;
				for (int i = 0; i < var.gpr.len; i++)
				{
					SgprState[gprIdx + i] = 0;
				}
			}
			else if (var.type == E_VarType::Vgpr)
			{
				int gprIdx = var.gpr.gprIdx;
				for (int i = 0; i < var.gpr.len; i++)
				{
					VgprState[gprIdx + i] = 0;
				}
			}
			else if (var.type == E_VarType::Agpr)
			{
				int gprIdx = var.gpr.gprIdx;
				for (int i = 0; i < var.gpr.len; i++)
				{
					AgprState[gprIdx + i] = 0;
				}
			}
			OperatorMap.erase(var.name);
		}
		void delVar(std::string var_name)
		{
			delVar(OperatorMap.find(var_name)->second);
		}
		void clrVar()
		{
			memset(SgprState, 0, sizeof(int)*MAX_SGPR_COUNT);
			memset(VgprState, 0, sizeof(int)*MAX_VGPR_COUNT);
			memset(AgprState, 0, sizeof(int)*MAX_AGPR_COUNT);
			OperatorMap.clear();
			ldsByteCount = 0;
		}

		T_Var getVar(std::string var_name)
		{
			return OperatorMap.find(var_name)->second;
		}

		std::string getVarStr(T_Var var, int len = 1)
		{
			if (var.type == E_VarType::Sgpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					if(var.gpr.isMinus)
						return std::string("-s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
					else
						return std::string("s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						if (var.gpr.isMinus)
							return std::string("-s[" + d2s(var.gpr.gprIdx) + "]");
						else
							return std::string("s[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						if (var.gpr.isMinus)
							return std::string("-s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
						else
							return std::string("s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Vgpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					if (var.gpr.isMinus)
						return std::string("-v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
					else
						return std::string("v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						if (var.gpr.isMinus)
							return std::string("-v[" + d2s(var.gpr.gprIdx) + "]");
						else
							return std::string("v[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						if (var.gpr.isMinus)
							return std::string("-v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
						else
							return std::string("v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Agpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					if (var.gpr.isMinus)
						return std::string("-acc[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
					else
						return std::string("acc[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						if (var.gpr.isMinus)
							return std::string("-acc[" + d2s(var.gpr.gprIdx) + "]");
						else
							return std::string("acc[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						if (var.gpr.isMinus)
							return std::string("-acc[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
						else
							return std::string("acc[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Imm || var.type == E_VarType::Macro)
			{
				return std::string(d2s(var.value));
			}
			else if (var.type == E_VarType::Laber)
			{
				return var.name;
			}

			return nullptr;
		}
		std::string getVarStr(std::string name, int len = 1)
		{
			if ((name == "vcc") || (name == "vcc_hi") || (name == "vcc_lo") ||
				(name == "exec") || (name == "exec_hi") || (name == "exec_lo") ||
				(name == "scc") ||
				(name == "off") || (name == "m0"))
			{
				return name;
			}
			if ((name[0] == '0') && (name[1] == 'x'))
			{
				return name;
			}

			return nullptr;
		}
		std::string getVarStr(double immVal, int len = 1)
		{
			if ((immVal - (int)immVal) != 0)
				return f2s((float)immVal);
			else
				return d2s((int)immVal);
		}
		
		std::string getfVarStr(T_Var var, int len = 1)
		{
			if (var.type == E_VarType::Sgpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					return std::string("s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						return std::string("s[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						return std::string("s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Vgpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					return std::string("v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						return std::string("v[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						return std::string("v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Imm || var.type == E_VarType::Macro)
			{
				return std::string(d2s(var.value));
			}
			else if (var.type == E_VarType::Laber)
			{
				return var.name;
			}

			return nullptr;
		}
		std::string getfVarStr(std::string name, int len = 1)
		{
			if ((name == "vcc") || (name == "vcc_hi") || (name == "vcc_lo") ||
				(name == "exec") || (name == "exec_hi") || (name == "exec_lo") ||
				(name == "off") || (name == "m0"))
			{
				return name;
			}
			if ((name[0] == '0') && (name[1] == 'x'))
			{
				return name;
			}

			return nullptr;
		}
		std::string getfVarStr(double immVal, int len = 1)
		{
			return f2s((float)immVal);
		}

		std::string gethVarStr(T_Var var, int len = 1)
		{
			if (var.type == E_VarType::Sgpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					return std::string("s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						return std::string("s[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						return std::string("s[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Vgpr)
			{
				if (var.gpr.forceLenEn == true)
				{
					var.gpr.forceLenEn = false;
					return std::string("v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + var.gpr.forceLen - 1) + "]");
				}
				else
				{
					if (len == 1)
					{
						return std::string("v[" + d2s(var.gpr.gprIdx) + "]");
					}
					else
					{
						return std::string("v[" + d2s(var.gpr.gprIdx) + ":" + d2s(var.gpr.gprIdx + len - 1) + "]");
					}
				}
			}
			else if (var.type == E_VarType::Imm || var.type == E_VarType::Macro)
			{
				return std::string(d2s(var.value));
			}
			else if (var.type == E_VarType::Laber)
			{
				return var.name;
			}

			return nullptr;
		}
		std::string gethVarStr(std::string name, int len = 1)
		{
			if ((name == "vcc") || (name == "vcc_hi") || (name == "vcc_lo") ||
				(name == "exec") || (name == "exec_hi") || (name == "exec_lo") ||
				(name == "off") || (name == "m0"))
			{
				return name;
			}
			if ((name[0] == '0') && (name[1] == 'x'))
			{
				return name;
			}

			return nullptr;
		}
		std::string gethVarStr(long immVal, int len = 1)
		{
			return d2hs((long)immVal);
		}

		E_VarType getVarType(T_Var var) { return var.type; }
		E_VarType getVarType(std::string name)
		{
			if ((name == "vcc") || (name == "vcc_hi") || (name == "vcc_lo") ||
				(name == "exec") || (name == "exec_hi") || (name == "exec_lo"))
			{
				return E_VarType::String;
			}
			else if (name == "off")
			{
				return E_VarType::Off;
			}

			return E_VarType::Off;
		}
		E_VarType getVarType(double immVal) { return E_VarType::Imm; }

		int getVarVal(int immVal) { return immVal; }
		double getVarVal(double immVal) { return immVal; }
		int getVarVal(T_Var reg) { return reg.gpr.gprIdx; }

		E_ReturnState ldsAllocByte(size_t groupLdsByte)
		{
			if (ldsByteCount + groupLdsByte > MAX_LDS_SIZE)
				return E_ReturnState::RTN_ERR;
			ldsByteCount += groupLdsByte;
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState ldsAllocDword(size_t groupLdsDword)
		{
			size_t lds_size_byte = groupLdsDword * 4;
			if (ldsByteCount + lds_size_byte > MAX_LDS_SIZE)
				return E_ReturnState::RTN_ERR;
			ldsByteCount += lds_size_byte;
			return E_ReturnState::SUCCESS;
		}
		/************************************************************************/
		/* 通用函数		                                                        */
		/************************************************************************/
		int tableCnt = 0;
		std::string sblk()
		{
			std::string str = "";
			for (int i = 0; i < tableCnt; i++)
			{
				str.append("    ");
			}
			return str;
		}
		void indent()
		{
			tableCnt++;
		}
		void backSpace()
		{
			tableCnt--;
		}
		void setTable(int tab)
		{
			tableCnt = tab;
		}

		void wrString(std::string str)
		{
			std::string tmp = sblk();
			tmp.append(str);
			tmp.append("\n");
			kernelString.append(tmp);
		}
		void wrLine(std::string line)
		{
			std::string tmp = sblk();
			tmp.append(line);
			tmp.append("\n");
			kernelString.append(tmp);
		}
		void clearString()
		{
			kernelString.clear();
		}

		void wrCommom1(std::string common)
		{
			kernelString.append("/************************************************************************************/\n");
			kernelString.append("/* " + common + " */\n");
			kernelString.append("/************************************************************************************/\n");
		}
		void wrCommom2(std::string common)
		{
			kernelString.append("// ==================================================================================\n");
			kernelString.append("// " + common + " \n");
			kernelString.append("// ==================================================================================\n");
		}
		void wrCommom3(std::string common)
		{
			kernelString.append("// ----------------------------------------------------------------------------------\n");
			kernelString.append("// " + common + " \n");
			kernelString.append("// ----------------------------------------------------------------------------------\n");
		}
		void wrComment4(std::string comment)
		{
			wrLine("// " + comment);
		}
		void wrComment5(std::string comment)
		{
			kernelString.pop_back();
			kernelString.append("\t// " + comment + "\n");
		}

		void wrLaber(T_Var lab)
		{
			int saveTabCnt = tableCnt;
			tableCnt = 0;
			wrLine(getVarStr(lab) + ":");
			tableCnt = saveTabCnt;
		}

		E_OpType getOp(std::string op)
		{
			if (op == "+") return E_OpType::ADD;
			if (op == "++") return E_OpType::INC;
			if (op == "-") return E_OpType::SUB;
			if (op == "--") return E_OpType::DEC;
			if (op == "==") return E_OpType::CMPEQ;
			if (op == "!=") return E_OpType::CMPNE;
			if (op == ">=") return E_OpType::CMPGE;
			if (op == ">") return E_OpType::CMPGT;
			if (op == "<=") return E_OpType::CMPLE;
			if (op == "<") return E_OpType::CMPLT;
			return E_OpType::NONE;
		}
#pragma region OCL_COMPILER
		/************************************************************************/
		/* ocl表达式编译器                                                        */
		/************************************************************************/
		char * src_pos;
		T_Token currToken;
		std::vector<std::string> keyWords;
		std::map<std::string, T_Var> ffccTempVar;

		// 词法分析器
		void next()
		{
			currToken.type = E_Token::None;
			currToken.valChar = 0;
			currToken.valInt = 0;
			currToken.valStr.clear();

			// 跳过空白
			while (*src_pos == ' ' || *src_pos == '\t')
				src_pos++;

			// 符号
			if ((*src_pos >= 'a' && *src_pos <= 'z') || (*src_pos >= 'A' && *src_pos <= 'Z') || (*src_pos == '_'))
			{
				int name_len = 0;
				while ((*src_pos >= 'a' && *src_pos <= 'z')
					|| (*src_pos >= 'A' && *src_pos <= 'Z')
					|| (*src_pos >= '0' && *src_pos <= '9') || (*src_pos == '_'))
				{
					src_pos++;
					name_len++;
				}
				currToken.valStr = std::string(src_pos - name_len, name_len);

				// 关键字
				for (std::string key : keyWords)
				{
					if (currToken.valStr == key)
					{
						if (currToken.valStr == "free")
						{
							currToken.type = E_Token::Free;
						}
						else
						{
							currToken.type = E_Token::KeyType;
						}
						return;
					}
				}

				// 符号
				currToken.type = E_Token::Symble;
			}
			// 数字
			else if (*src_pos >= '0' && *src_pos <= '9')
			{
				while (*src_pos >= '0' && *src_pos <= '9')
				{
					currToken.type = E_Token::Num;
					currToken.valInt = currToken.valInt * 10 + *src_pos - '0';
					src_pos++;
				}
				currToken.type = E_Token::Num;
			}
			// 宏定义
			else if (*src_pos == '#')
			{
				if (*(src_pos + 1) == 'd' &&*(src_pos + 2) == 'e' &&*(src_pos + 3) == 'f' &&
					*(src_pos + 4) == 'i'&& *(src_pos + 5) == 'n' && *(src_pos + 6) == 'e')
				{
					src_pos += 7;
					currToken.type = E_Token::Def;
					return;
				}
			}
			// 算数/逻辑运算
			else if (*src_pos == '+')
			{
				src_pos++;
				if (*src_pos == '+')
				{
					src_pos++;
					currToken.type = E_Token::Inc;
				}
				else
				{
					currToken.type = E_Token::Add;
				}
			}
			else if (*src_pos == '-')
			{
				src_pos++;
				if (*src_pos == '-')
				{
					src_pos++;
					currToken.type = E_Token::Dec;
				}
				else
				{
					currToken.type = E_Token::Sub;
				}
			}
			else if (*src_pos == '*')
			{
				currToken.type = E_Token::Mul;
				src_pos++;
			}
			else if (*src_pos == '/')
			{
				src_pos++;
				if (*src_pos == '/')
				{
					while (*src_pos != 0 && *src_pos != '\n')
						src_pos++;
				}
				else
				{
					currToken.type = E_Token::Div;
				}
			}
			else if (*src_pos == '%')
			{
				currToken.type = E_Token::Mod;
				src_pos++;
			}
			else if (*src_pos == '&')
			{
				src_pos++;
				if (*src_pos == '&')
				{
					src_pos++;
					currToken.type = E_Token::Lan;
				}
				else
				{
					currToken.type = E_Token::And;
				}
			}
			else if (*src_pos == '|')
			{
				src_pos++;
				if (*src_pos == '|')
				{
					src_pos++;
					currToken.type = E_Token::Lor;
				}
				else
				{
					currToken.type = E_Token::Or;
				}
			}
			else if (*src_pos == '^')
			{
				currToken.type = E_Token::Xor;
				src_pos++;
			}
			else if (*src_pos == '!')
			{
				src_pos++;
				if (*src_pos == '=')
				{
					src_pos++;
					currToken.type = E_Token::Ne;
				}
			}
			else if (*src_pos == '=')
			{
				src_pos++;
				if (*src_pos == '=')
				{
					src_pos++;
					currToken.type = E_Token::Eq;
				}
				else
				{
					currToken.type = E_Token::Assign;
				}
			}
			else if (*src_pos == '<')
			{
				src_pos++;
				if (*src_pos == '=')
				{
					src_pos++;
					currToken.type = E_Token::Le;
				}
				else if (*src_pos == '<')
				{
					src_pos++;
					currToken.type = E_Token::Shl;
				}
				else
				{
					currToken.type = E_Token::Lt;
				}
			}
			else if (*src_pos == '>')
			{
				src_pos++;
				if (*src_pos == '=')
				{
					src_pos++;
					currToken.type = E_Token::Ge;
				}
				else if (*src_pos == '>')
				{
					src_pos++;
					currToken.type = E_Token::Shr;
				}
				else
				{
					currToken.type = E_Token::Gt;
				}
			}
			// 其他符号
			else if (*src_pos == '\0')
			{
				currToken.type = E_Token::EndProg;
				currToken.valChar = *src_pos;
				src_pos++;
			}
			else if (*src_pos == '\n')
			{
				currToken.type = E_Token::EndLine;
				currToken.valChar = *src_pos;
				src_pos++;
			}
			else if (*src_pos == ';')
			{
				currToken.type = E_Token::EndExpr;
				currToken.valChar = *src_pos;
				src_pos++;
			}
			else if (*src_pos == ',')
			{
				currToken.type = E_Token::EndSymb;
				currToken.valChar = *src_pos;
				src_pos++;
			}
			else if (*src_pos == '(')
			{
				currToken.type = E_Token::OpenExpr;
				src_pos++;

				char * tmpPos = src_pos;
				while (*tmpPos != ')')
					tmpPos++;
				currToken.valStr = std::string(src_pos, tmpPos - src_pos);

				// 强制类型转换
				for (std::string key : keyWords)
				{
					if (currToken.valStr == key)
					{
						currToken.type = E_Token::TypeTrans;
						src_pos = tmpPos;
						return;
					}
				}
			}
			else if (*src_pos == ')')
			{
				currToken.type = E_Token::CloseExpr;
				currToken.valChar = *src_pos;
				src_pos++;
			}
			else if (*src_pos == '?' || *src_pos == ':')
			{
				currToken.type = E_Token::Other;
				currToken.valChar = *src_pos;
				src_pos++;
			}
			else
			{
				src_pos++;
			}

			return;
		}

		T_Var findTempVar(std::string name)
		{
			T_Var var;
			var.type = E_VarType::Off;
			if(ffccTempVar.find(name) != ffccTempVar.end())
				var = ffccTempVar.find(name)->second;
			return var;
		}
		void varAssign(T_Var leftVar, T_Var rightVar)
		{
			// 此时传入的leftVar携带变量名和寄存器长度信息
			if (rightVar.type == E_VarType::Sgpr || 
				rightVar.type == E_VarType::Macro || rightVar.type == E_VarType::Imm)
			{
				leftVar = newSgpr(leftVar.name, leftVar.gpr.len, leftVar.gpr.len);
				if (leftVar.gpr.len == 1)
				{
					op2("s_mov_b32", leftVar, rightVar);
				}
				else if (leftVar.gpr.len == 2)
				{
					if (rightVar.gpr.len == 1)
					{
						op2("s_mov_b32", leftVar, rightVar);
						op2("s_mov_b32", leftVar + 1, 0);
					}
					else
					{

						op2("s_mov_b32", leftVar, rightVar);
						op2("s_mov_b32", leftVar + 1, rightVar + 1);
					}
				}
			}
			else if (rightVar.type == E_VarType::Vgpr)
			{
				leftVar = newVgpr(leftVar.name, leftVar.gpr.len, leftVar.gpr.len);
				if (leftVar.gpr.len == 1)
				{
					op2("v_mov_b32", leftVar, rightVar);
				}
				else if (leftVar.gpr.len == 2)
				{
					if (rightVar.gpr.len == 1)
					{
						op2("v_mov_b32", leftVar, rightVar);
						op2("v_mov_b32", leftVar + 1, 0);
					}
					else
					{

						op2("v_mov_b32", leftVar, rightVar);
						op2("v_mov_b32", leftVar + 1, rightVar + 1);
					}
				}
			}

			wrComment5(leftVar.name + " = " + rightVar.name);
		}
		T_Var varAdd(T_Var var1, T_Var var2)
		{
			std::string exprName;
			T_Var rsltVar;
			rsltVar.type = E_VarType::None;
			rsltVar.gpr.len = 0;
			rsltVar.name = "";

			// 两个操作数均为数值,则直接计算结果,并返回
			if (((var1.type == E_VarType::Macro) || (var1.type == E_VarType::Imm)) &&
				((var2.type == E_VarType::Macro) || (var2.type == E_VarType::Imm)))
			{
				rsltVar.type = E_VarType::Imm;
				rsltVar.value = var1.value + var2.value;
				rsltVar.name = std::to_string(rsltVar.value);
			}

			// 如果两个操作数均为SGRP 或者一个为SGPR一个为数值
			else if ((var1.type == E_VarType::Sgpr && var2.type == E_VarType::Sgpr) ||
				(var1.type == E_VarType::Sgpr && (var2.type == E_VarType::Macro || var2.type == E_VarType::Imm)) ||
				(var2.type == E_VarType::Sgpr && (var1.type == E_VarType::Macro || var1.type == E_VarType::Imm)))
			{
				exprName = var1.name + " + " + var2.name;

				if (ffccTempVar.find(exprName) != ffccTempVar.end())
				{
					rsltVar = ffccTempVar.find(exprName)->second;
				}
				else
				{
					if (var1.gpr.len == 2 && var2.gpr.len == 2)
					{
						rsltVar = newSgpr(exprName, 2);
						op3("s_add_u32", rsltVar, var1, var2);
						op3("s_addc_u32", rsltVar + 1, var1 + 1, var2 + 1);
					}
					else if (var1.gpr.len == 2 && var2.gpr.len == 1)
					{
						rsltVar = newSgpr(exprName, 2);
						op3("s_add_u32", rsltVar, var1, var2);
						op3("s_addc_u32", rsltVar + 1, var1 + 1, 0);
					}
					else if (var1.gpr.len == 1 && var2.gpr.len == 2)
					{
						rsltVar = newSgpr(exprName, 2);
						op3("s_add_u32", rsltVar, var1, var2);
						op3("s_addc_u32", rsltVar + 1, 0, var2 + 1);
					}
					else
					{
						rsltVar = newSgpr(exprName);
						op3("s_add_u32", rsltVar, var1, var2);
					}
				}
			}

			// 如果有VGPR参与
			else if (var1.type == E_VarType::Vgpr || var2.type == E_VarType::Vgpr)
			{
				exprName = var1.name + " + " + var2.name;

				if (ffccTempVar.find(exprName) != ffccTempVar.end())
				{
					rsltVar = ffccTempVar.find(exprName)->second;
				}
				else
				{
					// 如果有立即数,则立即数必须在第一个操作数
					if (var2.type == E_VarType::Macro || var2.type == E_VarType::Imm)
					{
						if (var1.gpr.len == 2)
						{
							rsltVar = newVgpr(exprName, 2, 2);
							v_addc_u32(rsltVar, var2, var1);
							v_addc_co_u32(rsltVar[1], 0, var2[1]);
						}
						else
						{
							rsltVar = newVgpr(exprName);
							v_add_u32(rsltVar, var2, var1);
						}
					}
					else
					{
						if (var2.gpr.len == 2)
						{
							rsltVar = newVgpr(exprName, 2, 2);
							op2("v_mov_b32", rsltVar[1], var2[1]);
							v_addc_u32(rsltVar[0], var1[0], var2[0]);
							v_addc_co_u32(rsltVar[1], rsltVar[1], var1[1]);
						}
						else if (var1.gpr.len == 2)
						{
							rsltVar = newVgpr(exprName, 2, 2);
							op2("v_mov_b32", rsltVar[1], var1[1]);
							v_addc_u32(rsltVar[0], var1[0], var2[0]);
							if (var2.gpr.len == 2)
							{
								v_addc_co_u32(rsltVar[1], rsltVar[1], var2[1]);
							}
							else
							{
								v_addc_co_u32(rsltVar[1], 0, rsltVar[1]);
							}
						}
						else
						{
							rsltVar = newVgpr(exprName);
							v_add_u32(rsltVar, var1, var2);
						}
					}
				}
			}

			if (rsltVar.type != E_VarType::Imm)
			{
				wrComment5(exprName);
				ffccTempVar.insert(std::pair<std::string, T_Var>(rsltVar.name, rsltVar));
			}

			return rsltVar;
		}
		T_Var varSub(T_Var var1, T_Var var2)
		{
			T_Var rsltVar;
			rsltVar.type = E_VarType::None;
			rsltVar.gpr.len = 0;
			rsltVar.name = "";

			return rsltVar;
		}
		T_Var varMul(T_Var var1, T_Var var2)
		{
			std::string exprName;
			T_Var rsltVar;
			rsltVar.type = E_VarType::None;
			rsltVar.gpr.len = 0;
			rsltVar.name = "";

			// 两个操作数均为数值,则直接计算结果,并返回
			if ((var1.type == E_VarType::Macro || var1.type == E_VarType::Imm) &&
				(var2.type == E_VarType::Macro || var2.type == E_VarType::Imm))
			{
				rsltVar.type = E_VarType::Imm;
				rsltVar.value = var1.value * var2.value;
				rsltVar.name = std::to_string(rsltVar.value);
			}

			// 如果两个操作数均为SGRP 或者一个为SGPR一个为数值
			else if ((var1.type == E_VarType::Sgpr && var2.type == E_VarType::Sgpr) ||
				(var1.type == E_VarType::Sgpr && (var2.type == E_VarType::Macro || var2.type == E_VarType::Imm)) ||
				(var2.type == E_VarType::Sgpr && (var1.type == E_VarType::Macro || var1.type == E_VarType::Imm)))
			{
				exprName = var1.name + " * " + var2.name;

				if (ffccTempVar.find(exprName) != ffccTempVar.end())
				{
					rsltVar = ffccTempVar.find(exprName)->second;
				}
				else
				{
					rsltVar = newSgpr(exprName);
					op3("s_mul_i32", rsltVar, var1, var2);
				}
			}

			else if (var1.type == E_VarType::Vgpr || var2.type == E_VarType::Vgpr)
			{
				exprName = var1.name + " * " + var2.name;

				if (ffccTempVar.find(exprName) != ffccTempVar.end())
				{
					rsltVar = ffccTempVar.find(exprName)->second;
				}
				else
				{
					// 如果有立即数,则立即数必须在第一个操作数
					if (var2.type == E_VarType::Macro || var2.type == E_VarType::Imm)
					{
						rsltVar = newVgpr(exprName);
						op3("v_mul_u32_u24", rsltVar, var2, var1);
					}
					else
					{
						rsltVar = newVgpr(exprName);
						op3("v_mul_u32_u24", rsltVar, var1, var2);
					}
				}
			}

			if (rsltVar.type != E_VarType::Imm)
			{
				wrComment5(exprName);
				ffccTempVar.insert(std::pair<std::string, T_Var>(rsltVar.name, rsltVar));
			}

			return rsltVar;
		}
		T_Var varDiv(T_Var var1, T_Var var2)
		{
			T_Var divRslt, modRslt;
			divRslt.type = modRslt.type = E_VarType::None;
			divRslt.gpr.len = modRslt.gpr.len = 0;
			divRslt.name = modRslt.name = "";

			// 两个操作数均为数值,则直接计算结果,并返回
			if ((var1.type == E_VarType::Macro || var1.type == E_VarType::Imm) &&
				(var2.type == E_VarType::Macro || var2.type == E_VarType::Imm))
			{
				divRslt.type = E_VarType::Imm;
				divRslt.value = var1.value / var2.value;
				divRslt.name = std::to_string(divRslt.value);
			}
			else
			{
				if (ffccTempVar.find(var1.name + " / " + var2.name) != ffccTempVar.end())
				{
					divRslt = ffccTempVar.find(var1.name + " / " + var2.name)->second;
				}
				else
				{
					divRslt = newVgpr(var1.name + " / " + var2.name);
					modRslt = newVgpr(var1.name + " % " + var2.name);
					fv_div_u32(var1, var2, divRslt, modRslt);
				}
			}

			if (divRslt.type != E_VarType::Imm)
			{
				wrComment5(divRslt.name);
				ffccTempVar.insert(std::pair<std::string, T_Var>(divRslt.name, divRslt));
				ffccTempVar.insert(std::pair<std::string, T_Var>(modRslt.name, modRslt));
			}

			return divRslt;
		}
		T_Var varMod(T_Var var1, T_Var var2)
		{
			T_Var divRslt, modRslt;
			divRslt.type = modRslt.type = E_VarType::None;
			divRslt.gpr.len = modRslt.gpr.len = 0;
			divRslt.name = modRslt.name = "";

			// 两个操作数均为数值,则直接计算结果,并返回
			if ((var1.type == E_VarType::Macro || var1.type == E_VarType::Imm) &&
				(var2.type == E_VarType::Macro || var2.type == E_VarType::Imm))
			{
				modRslt.type = E_VarType::Imm;
				modRslt.value = var1.value % var2.value;
				modRslt.name = std::to_string(modRslt.value);
			}
			else
			{
				if (ffccTempVar.find(var1.name + " % " + var2.name) != ffccTempVar.end())
				{
					modRslt = ffccTempVar.find(var1.name + " % " + var2.name)->second;
				}
				else
				{
					divRslt = newVgpr(var1.name + " / " + var2.name);
					modRslt = newVgpr(var1.name + " % " + var2.name);
					fv_div_u32(var1, var2, divRslt, modRslt);
				}
			}

			if (modRslt.type != E_VarType::Imm)
			{
				wrComment5(modRslt.name);
				ffccTempVar.insert(std::pair<std::string, T_Var>(divRslt.name, divRslt));
				ffccTempVar.insert(std::pair<std::string, T_Var>(modRslt.name, modRslt));
			}

			return modRslt;
		}

		// 递归下降
		T_Var level3Op()
		{
			T_Var rtnVar;

			if (currToken.type == E_Token::OpenExpr)
			{
				while (true)
				{
					next();

					rtnVar = expressionProc();

					if (currToken.type == E_Token::CloseExpr)
					{
						next();
						break;
					}
				}

			}
			else if (currToken.type == E_Token::Num)
			{
				// 返回数值,可直接用于计算
				rtnVar.type = E_VarType::Imm;
				rtnVar.value = currToken.valInt;
				rtnVar.name = std::to_string(rtnVar.value);
				rtnVar.gpr.len = 0;

				next();
			}
			else if (currToken.type == E_Token::Symble)
			{
				if (OperatorMap.find(currToken.valStr) == OperatorMap.end())
				{
					printf("错误: 变量%s 未定义!\n",currToken.valStr.c_str());
					rtnVar.type = E_VarType::None;
					return rtnVar;
				}
				rtnVar = OperatorMap.find(currToken.valStr)->second;

				next();
			}

			return rtnVar;
		}
		T_Var level2Op()
		{
			T_Var var1, var2, rsltVar;

			rsltVar = var1 = level3Op();

			if (currToken.type == E_Token::Mul)
			{
				next();
				var2 = level3Op();

				rsltVar = varMul(var1, var2);
			}
			else if (currToken.type == E_Token::Div)
			{
				next();
				var2 = level3Op();

				rsltVar = varDiv(var1, var2);
			}
			else if (currToken.type == E_Token::Mod)
			{
				next();
				var2 = level3Op();

				rsltVar = varMod(var1, var2);
			}

			return rsltVar;
		}
		T_Var level1Op()
		{
			T_Var var1, var2, rsltVar;

			rsltVar = var1 = level2Op();

			if (currToken.type == E_Token::Add)
			{
				next();
				var2 = level2Op();

				rsltVar = varAdd(var1, var2);
			}
			else if (currToken.type == E_Token::Sub)
			{
				next();
				var2 = level2Op();

				rsltVar = varSub(var1, var2);
			}

			return rsltVar;
		}
		
		// 表达式计算
		T_Var expressionProc()
		{
			return level1Op();
		}

		// 语句分析
		void statementProc()
		{
			// 宏定义
			if (currToken.type == E_Token::Def)
			{
				T_Var rightVar;
				T_Var varNew;
				varNew.type = E_VarType::Macro;

				// 读取define名
				next();
				varNew.name = currToken.valStr;

				next(); // 读取define值(应计算表达式)
				rightVar = expressionProc();

				varNew.value = rightVar.value;
				varNew.gpr.len = 0;
				OperatorMap.insert(std::pair<std::string, T_Var>(varNew.name, varNew));
				//printf("define: %s = %d\n", varNew.name.c_str(), varNew.value);

				// 因为表达式计算会读取到下一个token，所以如果退出到上层的while循环，会多读一个token
				if (currToken.type != E_Token::EndExpr)
				{
					statementProc();
				}
			}
			// 变量释放
			else if (currToken.type == E_Token::Free)
			{
				while (true)
				{
					// 读取变量名
					next();
					if (currToken.type == E_Token::EndExpr)
					{
						// 释放未在ocl中被声明的全部临时寄存器
						for (auto var : ffccTempVar)
						{
							delVar(var.second);
						}
						break;
					}
					else
					{
						delVar(getVar(currToken.valStr));

						next();
						if (currToken.type == E_Token::EndExpr)
							break;
					}
				}
			}
			// 变量声明
			else if (currToken.type == E_Token::KeyType)
			{
				T_Var varNew;
				varNew.value = 0;
				varNew.type = E_VarType::Vgpr;
				varNew.gpr.len = 0;
				if (currToken.valStr == "uint")			varNew.gpr.len = 1;
				if (currToken.valStr == "ulong")		varNew.gpr.len = 2;

				while (true)
				{
					next(); // 读取变量名
					varNew.name = currToken.valStr;

					if (currToken.type == E_Token::EndExpr)
						break;

					// 检查重复定义
					if (OperatorMap.find(varNew.name) != OperatorMap.end())
					{
						printf("错误: 变量%s 重复定义!\n",varNew.name.c_str());
						return;
					}

					// 变量赋值
					next();
					if (currToken.type == E_Token::Assign)
					{
						// 计算表达式并赋值
						next();
						T_Var rightVar = expressionProc();

						varAssign(varNew, rightVar);

						// 因为表达式计算会读取到下一个token，所以如果是宏定义的表达式结束，如果退出到上层的while循环，会多读一个token
						if (currToken.type != E_Token::EndExpr)
						{
							statementProc();
						}
						else
						{
							break;
						}
					}

					if (currToken.type == E_Token::EndExpr)
					{
						OperatorMap.insert(std::pair<std::string, T_Var>(varNew.name, varNew));
						printf("declare: %s = %d\n", varNew.name.c_str(), varNew.value);
						break;
					}
					else if (currToken.type == E_Token::EndSymb)
					{
						OperatorMap.insert(std::pair<std::string, T_Var>(varNew.name, varNew));
						printf("declare: %s = %d\n", varNew.name.c_str(), varNew.value);
					}
				}
			}
			// 表达式计算
			else if (currToken.type == E_Token::Symble)
			{
				T_Var leftVar, rightVar;

				// 检查左值是否定义
				if (OperatorMap.find(currToken.valStr) == OperatorMap.end())
				{
					printf("错误: 变量%s 未定义!\n", currToken.valStr.c_str());
					return;
				}
				leftVar = OperatorMap.find(currToken.valStr)->second;

				// 读取等号,判断表达式类型
				next();
				if (currToken.type != E_Token::Assign)
				{
					printf("错误: 不支持单目运算!\n");
					return;
				}

				while (true)
				{
					next();

					rightVar = expressionProc();

					if (currToken.type == E_Token::EndExpr)
						break;
				}

				if ((rightVar.type == E_VarType::Imm) || (rightVar.type == E_VarType::Macro))
				{
					printf("expression: %s = %d\n", leftVar.name.c_str(), rightVar.value);
				}
				else
				{
					printf("expression: %s = %s\n", leftVar.name.c_str(), rightVar.name.c_str());
				}
			}
		}

		bool ffcc(std::string src_str)
		{
			keyWords.clear();
			keyWords.push_back("uint");
			keyWords.push_back("ulong");
			keyWords.push_back("free");
			ffccTempVar.clear();

			src_pos = &src_str[0];

			while (true)
			{
				next();

				if (currToken.type == E_Token::EndProg)
				{
					// 只释放未在ocl中被声明的临时寄存器
					for (auto var : ffccTempVar)
					{
						delVar(var.second);
					}

					return true;
				}

				statementProc();
			}
		}
#pragma endregion

#pragma region ISA_REGION
		/************************************************************************************/
		/* SMEM																				*/
		/************************************************************************************/
		template <typename T>
		E_ReturnState s_load_dword(int num, T_Var s_dst, T_Var s_base, T offset, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("s_load_dword");
			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			if (num == 1)
			{
				str.append(getVarStr(s_dst));
			}
			else
			{
				str.append(getVarStr(s_dst, num));
			}
			str.append(", ");
			str.append(getVarStr(s_base, 2));
			str.append(", ");
			str.append(getVarStr(offset));

			if (glc == true)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			if (s_dst.type != E_VarType::Sgpr)
			{
				str.append("dest reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState s_store_dword(int num, T_Var s_dst, T_Var s_base, T offset, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("s_store_dword");
			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			if (num == 1)
			{
				str.append(getVarStr(s_dst));
			}
			else
			{
				str.append(getVarStr(s_dst, num));
			}
			str.append(", ");
			str.append(getVarStr(s_base, 2));
			str.append(", ");
			str.append(getVarStr(offset));

			if (glc == true)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			if (s_dst.type != E_VarType::Sgpr)
			{
				str.append("dest reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState s_atomic_op(E_OpType op, T_Var s_dat, T_Var s_addr, T offset, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("s_atomic_");
			switch (op)
			{
			case E_OpType::ADD:		str.append("add");			break;
			case E_OpType::INC:		str.append("inc");			break;
			case E_OpType::DEC:		str.append("dec");			break;
			case E_OpType::SUB:		str.append("sub");			break;
			case E_OpType::AND:		str.append("and");			break;
			case E_OpType::OR:		str.append("or");			break;
			case E_OpType::XOR:		str.append("xor");			break;
			case E_OpType::SMAX:	str.append("smax");			break;
			case E_OpType::UMAX:	str.append("umax");			break;
			case E_OpType::SMIN:	str.append("smin");			break;
			case E_OpType::UMIN:	str.append("umin");			break;
			case E_OpType::SWAP:	str.append("swap");			break;
			case E_OpType::CMPSWAP:	str.append("cmpswap");		break;
			default:				str.append("invalid op");	return E_ReturnState::RTN_ERR;
			}

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(s_dat));
			str.append(", ");
			str.append(getVarStr(s_addr, 2));
			str.append(", ");
			str.append(getVarStr(offset));

			if (glc == true)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			if (s_dat.type != E_VarType::Sgpr)
			{
				str.append("dest reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState s_atomic_op2(E_OpType op, T_Var s_dat, T_Var s_addr, T offset, bool glc = true)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("s_atomic_");
			switch (op)
			{
			case E_OpType::ADD:		str.append("add");			break;
			case E_OpType::INC:		str.append("inc");			break;
			case E_OpType::DEC:		str.append("dec");			break;
			case E_OpType::SUB:		str.append("sub");			break;
			case E_OpType::AND:		str.append("and");			break;
			case E_OpType::OR:		str.append("or");			break;
			case E_OpType::XOR:		str.append("xor");			break;
			case E_OpType::SMAX:	str.append("smax");			break;
			case E_OpType::UMAX:	str.append("umax");			break;
			case E_OpType::SMIN:	str.append("smin");			break;
			case E_OpType::UMIN:	str.append("umin");			break;
			case E_OpType::SWAP:	str.append("swap");			break;
			case E_OpType::CMPSWAP:	str.append("cmpswap");		break;
			default:				str.append("invalid op");	return E_ReturnState::RTN_ERR;
			}

			str.append("_x2");
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(s_dat, 2));
			str.append(", ");
			str.append(getVarStr(s_addr, 2));
			str.append(", ");
			str.append(getVarStr(offset));

			if (glc != true)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			if (s_dat.type != E_VarType::Sgpr)
			{
				str.append("dest reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}

		/************************************************************************************/
		/* MUBUF																			*/
		/************************************************************************************/
		template <typename T>
		E_ReturnState buffer_load_dword(
			int num, T_Var v_dst,
			T_Var v_offset_idx,
			T_Var s_desc,
			T s_base_offset,
			bool idx_en, bool off_en, bool lds_en,
			unsigned int i_offset)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("buffer_load_dword");
			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// dest_data
			if (num == 1)
			{
				str.append(getVarStr(v_dst));
			}
			else
			{
				str.append(getVarStr(v_dst, num));
			}

			// v_index & v_offset
			str.append(", ");
			if ((idx_en == false) && (off_en == false))
			{
				str.append(getVarStr("off"));
			}
			else if ((idx_en == false) && (off_en == true))
			{
				str.append(getVarStr(v_offset_idx));
			}
			else if ((idx_en == true) && (off_en == false))
			{
				str.append(getVarStr(v_offset_idx));
			}
			else if ((idx_en == true) && (off_en == true))
			{
				str.append(getVarStr(v_offset_idx, 2));
			}

			// s_buffer_descripter
			str.append(", ");
			str.append(getVarStr(s_desc, 4));

			// s_base_offset
			str.append(", ");
			str.append(getVarStr(s_base_offset));

			if (idx_en == true)
			{
				str.append(" ");
				str.append("idxen");
			}
			if (off_en == true)
			{
				str.append(" ");
				str.append("offen");
			}

			// inst_offset
			str.append(" ");
			str.append("offset:" + d2s(i_offset));

			if (lds_en == true)
			{
				str.append(" ");
				str.append("lds");
			}

			wrLine(str);

			//error check 
			str = "";
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("dest reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((idx_en == true) || (off_en == true))
			{
				if (getVarType(v_offset_idx) != E_VarType::Vgpr)
				{
					str.append("offset and index reg not vgpr");
					wrLine(str);
					return E_ReturnState::RTN_ERR;
				}
			}
			if (s_desc.type != E_VarType::Sgpr)
			{
				str.append("buffer descriptor reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_desc.gpr.len != 4)
			{
				str.append("buffer descriptor reg not 4-dword");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			/*if (s_base_offset.type != E_VarType::Sgpr)
			{
				str.append("buffer obj base offset addr reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}*/
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset is not 12-bit unsigned int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((num > 1) && (lds_en == true))
			{
				str.append("lds direct only support 1dword");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			
			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState buffer_store_dword(
			int num, T_Var v_dst,
			T_Var v_offset_idx,
			T_Var s_desc,
			T s_base_offset,
			bool idx_en, bool off_en,
			unsigned int i_offset)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("buffer_store_dword");
			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// dest_data
			if (num == 1)
			{
				str.append(getVarStr(v_dst));
			}
			else
			{
				str.append(getVarStr(v_dst, num));
			}

			// v_index & v_offset
			str.append(", ");
			if ((idx_en == false) && (off_en == false))
			{
				str.append(getVarStr("off"));
			}
			else if ((idx_en == false) && (off_en == true))
			{
				str.append(getVarStr(v_offset_idx));
			}
			else if ((idx_en == true) && (off_en == false))
			{
				str.append(getVarStr(v_offset_idx));
			}
			else if ((idx_en == true) && (off_en == true))
			{
				str.append(getVarStr(v_offset_idx, 2));
			}

			// s_buffer_descripter
			str.append(", ");
			str.append(getVarStr(s_desc, 4));

			// s_base_offset
			str.append(", ");
			str.append(getVarStr(s_base_offset));

			if (idx_en == true)
			{
				str.append(" ");
				str.append("idxen");
			}
			if (off_en == true)
			{
				str.append(" ");
				str.append("offen");
			}

			// inst_offset
			str.append(" ");
			str.append("offset:" + d2s(i_offset));
			
			wrLine(str);

			//error check 
			str = "";
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("dest reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((idx_en == true) || (off_en == true))
			{
				if (getVarType(v_offset_idx) != E_VarType::Vgpr)
				{
					str.append("offset and index reg not vgpr");
					wrLine(str);
					return E_ReturnState::RTN_ERR;
				}
			}
			if (s_desc.type != E_VarType::Sgpr)
			{
				str.append("buffer descriptor reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_desc.gpr.len != 4)
			{
				str.append("buffer descriptor reg not 4-dword");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			/*if (s_base_offset.type != E_VarType::Sgpr)
			{
				str.append("buffer obj base offset addr reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}*/
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset is not 12-bit unsigned int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}

		/************************************************************************************/
		/* FLAT																				*/
		/************************************************************************************/
		template <typename T>
		E_ReturnState flat_load_dword(int num, T_Var v_dst, T_Var v_offset, T s_addr, int i_offset = 0, bool glc = false)
		{
			if (IsaArch >= E_IsaArch::Gfx900)
			{
				return flat_load_dword_gfx900(num, v_dst, v_offset, s_addr, i_offset, glc);
			}
			else
			{
				return flat_load_dword_gfx800(num, v_dst, v_offset, i_offset, glc);
			}
		}
		E_ReturnState flat_load_dword_gfx800(int num, T_Var v_dst, T_Var v_offset_addr, int i_offset = 0, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("flat_load_dword");
			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			if (num == 1)
			{
				str.append(getVarStr(v_dst));
			}
			else
			{
				str.append(getVarStr(v_dst, num));
			}
			str.append(", ");
			str.append(getVarStr(v_offset_addr, 2));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			str.append(" ");
			if (glc == true)
			{
				str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			if (!((num > 0) && (num <= 4)))
			{
				str.append("load data error");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState flat_load_dword_gfx900(int num, T_Var v_dst, T_Var v_offset_addr, T s_addr, int i_offset = 0, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";

			// op
			str.append("global_load_dword");
			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// v_dest
			if (num == 1)
			{
				str.append(getVarStr(v_dst));
			}
			else
			{
				str.append(getVarStr(v_dst, num));
			}
			str.append(", ");

			// v_offset(32-bit) / v_address(64-bit)
			bool is64AddrMode = false;
			if (getVarType(s_addr) == E_VarType::Off)
			{
				is64AddrMode = true;
			}
			// LLVM currently expects a 64-bit vaddr regardless of addressing mode. This have to be fixed.
			//if (is64AddrMode == true)		str.append(getVarStr(v_offset_addr, 2));
			//else							str.append(getVarStr(v_offset_addr));
			str.append(getVarStr(v_offset_addr, 2));
			str.append(", ");

			// s_address(32-bit) / "off"(64-bit)
			if (is64AddrMode == true)		str.append(getVarStr(s_addr));
			else							str.append(getVarStr(s_addr, 2));

			// imm_offset
			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			if (i_offset != 0)
			{
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}
			str.append(" ");
			if (glc == true)
			{
				str.append("glc");
			}

			wrLine(str);

			// error check
			if (!((num > 0) && (num <= 4)))
			{
				str.append("load data error");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2)&&(is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}

		template <typename T>
		E_ReturnState flat_store_dword(int num, T_Var v_offset_addr, T_Var v_dat, T s_addr, int i_offset = 0, bool glc = false)
		{
			if (IsaArch >= E_IsaArch::Gfx900)
			{
				return flat_store_dword_gfx900(num, v_offset_addr, v_dat, s_addr, i_offset, glc);
			}
			else
			{
				return flat_store_dword_gfx800(num, v_offset_addr, v_dat, i_offset, glc);
			}
		}
		E_ReturnState flat_store_dword_gfx800(int num, T_Var v_offset_addr, T_Var v_dat, int i_offset = 0, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append("flat_store_dword");

			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(v_offset_addr, 2));
			str.append(", ");
			if (num == 1)
			{
				str.append(getVarStr(v_dat));
			}
			else
			{
				str.append(getVarStr(v_dat, num));
			}

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			str.append(" ");
			if (glc == true)
			{
				str.append("glc");
			}

			wrLine(str);

			if (!((num > 0) && (num <= 4)))
			{
				str.append("load data error");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState flat_store_dword_gfx900(int num, T_Var v_offset_addr, T_Var v_dat, T s_addr, int i_offset = 0, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";

			// op
			str.append("global_store_dword");

			if (num > 1)
			{
				str.append("x" + d2s(num));
			}
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// v_offset(32-bit) / v_address(64-bit)
			bool is64AddrMode = false;
			if (getVarType(s_addr) == E_VarType::Off)
			{
				is64AddrMode = true;
			}
			// LLVM currently expects a 64-bit vaddr regardless of addressing mode. This have to be fixed.
			//if (is64AddrMode == true)		str.append(getVarStr(v_offset_addr, 2));
			//else							str.append(getVarStr(v_offset_addr));
			str.append(getVarStr(v_offset_addr, 2));
			str.append(", ");

			// v_data
			if (num == 1)
			{
				str.append(getVarStr(v_dat));
			}
			else
			{
				str.append(getVarStr(v_dat, num));
			}
			str.append(", ");

			// s_address(32-bit) / "off"(64-bit)
			if (is64AddrMode == true)		str.append(getVarStr(s_addr));
			else							str.append(getVarStr(s_addr, 2));

			// imm_offset
			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			str.append(" ");
			if (glc == true)
			{
				str.append("glc");
			}

			wrLine(str);

			// error check
			if (!((num > 0) && (num <= 4)))
			{
				str.append("load data error");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2) && (is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}

		template <typename T>
		E_ReturnState flat_atomic_op(E_OpType op, T_Var v_dst, T_Var v_offset_addr, T_Var* v_dat, T s_addr, int i_offset = 0, bool glc = false)
		{
			if (IsaArch >= E_IsaArch::Gfx900)
			{
				return flat_atomic_op_gfx900(op, v_dst, v_offset_addr, v_dat, s_addr, i_offset, glc);
			}
			else
			{
				//return flat_load_dword_gfx800(op, v_dst, v_offset, i_offset);
			}
		}
		template <typename T>
		E_ReturnState flat_atomic_op_gfx900(E_OpType op, T_Var v_dst, T_Var v_offset_addr, T_Var v_dat, T s_addr, int i_offset = 0, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";

			// op
			str.append("global_atomic_");
			switch (op)
			{
			case E_OpType::ADD:		str.append("add");			break;
			case E_OpType::INC:		str.append("inc");			break;
			case E_OpType::DEC:		str.append("dec");			break;
			case E_OpType::SUB:		str.append("sub");			break;
			case E_OpType::AND:		str.append("and");			break;
			case E_OpType::OR:		str.append("or");			break;
			case E_OpType::XOR:		str.append("xor");			break;
			case E_OpType::SMAX:	str.append("smax");			break;
			case E_OpType::UMAX:	str.append("umax");			break;
			case E_OpType::SMIN:	str.append("smin");			break;
			case E_OpType::UMIN:	str.append("umin");			break;
			case E_OpType::SWAP:	str.append("swap");			break;
			case E_OpType::CMPSWAP:	str.append("cmpswap");		break;
			default:				str.append("invalid op");	return E_ReturnState::RTN_ERR;
			}

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// return data
			str.append(getVarStr(v_dst));
			str.append(", ");

			// v_offset(32-bit) / v_address(64-bit)
			bool is64AddrMode = false;
			if (getVarType(s_addr) == E_VarType::Off)
			{
				is64AddrMode = true;
			}
			if (is64AddrMode == true)
			{
				str.append(getVarStr(v_offset_addr, 2));
			}
			else
			{
				str.append(getVarStr(v_offset_addr));
			}
			str.append(", ");

			// source data
			str.append(getVarStr(v_dat));
			str.append(", ");

			// s_address(32-bit) / "off"(64-bit)
			str.append(getVarStr(s_addr));
			
			// imm_offset
			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			if (i_offset != 0)
			{
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}
			if (glc == true)
			{
				str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			// error check
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2) && (is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		E_ReturnState flat_atomic_op_gfx800(E_OpType op, T_Var v_dst, T_Var v_addr, T_Var v_dat, int i_offset = 0, bool glc = false)
		{
			size_t tmpIdx;
			std::string str = "";

			// op
			str.append("flat_atomic_");
			switch (op)
			{
			case E_OpType::ADD:		str.append("add");			break;
			case E_OpType::INC:		str.append("inc");			break;
			case E_OpType::DEC:		str.append("dec");			break;
			case E_OpType::SUB:		str.append("sub");			break;
			case E_OpType::AND:		str.append("and");			break;
			case E_OpType::OR:		str.append("or");			break;
			case E_OpType::XOR:		str.append("xor");			break;
			case E_OpType::SMAX:	str.append("smax");			break;
			case E_OpType::UMAX:	str.append("umax");			break;
			case E_OpType::SMIN:	str.append("smin");			break;
			case E_OpType::UMIN:	str.append("umin");			break;
			case E_OpType::SWAP:	str.append("swap");			break;
			case E_OpType::CMPSWAP:	str.append("cmpswap");		break;
			default:				str.append("invalid op");	return E_ReturnState::RTN_ERR;
			}

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// return data
			str.append(getVarStr(v_dst));
			str.append(", ");

			// v_address
			str.append(getVarStr(v_addr, 2));
			str.append(", ");

			// source data
			str.append(getVarStr(v_dat));

			// imm_offset
			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			if (i_offset != 0)
			{
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}
			if (glc == true)
			{
				str.append(" ");
				str.append("glc");
			}

			wrLine(str);

			// error check
			if (v_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState flat_atomic_op2(E_OpType op, T_Var v_dst, T_Var v_offset_addr, T_Var v_dat, T s_addr, int i_offset = 0)
		{
			if (IsaArch >= E_IsaArch::Gfx900)
			{
				return flat_atomic_op2_gfx900(op, v_dst, v_offset_addr, v_dat, s_addr, i_offset);
			}
			else
			{
				//return flat_load_dword_gfx800(op, v_dst, v_offset, i_offset);
			}
		}
		template <typename T>
		E_ReturnState flat_atomic_op2_gfx900(E_OpType op, T_Var v_dst, T_Var v_offset_addr, T_Var v_dat, T s_addr, int i_offset = 0)
		{
			size_t tmpIdx;
			std::string str = "";

			// op
			str.append("global_atomic_");
			switch (op)
			{
			case E_OpType::ADD:		str.append("add");			break;
			case E_OpType::INC:		str.append("inc");			break;			
			case E_OpType::DEC:		str.append("dec");			break;
			case E_OpType::SUB:		str.append("sub");			break;
			case E_OpType::AND:		str.append("and");			break;
			case E_OpType::OR:		str.append("or");			break;
			case E_OpType::XOR:		str.append("xor");			break;
			case E_OpType::SMAX:	str.append("smax");			break;
			case E_OpType::UMAX:	str.append("umax");			break;
			case E_OpType::SMIN:	str.append("smin");			break;
			case E_OpType::UMIN:	str.append("umin");			break;
			case E_OpType::SWAP:	str.append("swap");			break;
			case E_OpType::CMPSWAP:	str.append("cmpswap");		break;
			default:				str.append("invalid op");	return E_ReturnState::RTN_ERR;
			}
			
			str.append("x2");
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			// return data
			str.append(getVarStr(v_dst, 2));
			str.append(", ");
			// v_offset(32-bit) / v_address(64-bit)
			bool is64AddrMode = false;
			if (getVarType(s_addr) == E_VarType::Off)
			{
				is64AddrMode = true;
			}
			if (is64AddrMode == true)
			{
				str.append(getVarStr(v_offset_addr, 2));
			}
			else
			{
				str.append(getVarStr(v_offset_addr));
			}
			str.append(", ");

			// s_address(32-bit) / "off"(64-bit)
			str.append(getVarStr(s_addr));

			// imm_offset
			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			wrLine(str);

			// error check
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2) && (is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		
		/************************************************************************************/
		/* DS																				*/
		/************************************************************************************/
		E_ReturnState ds_read_dword(int num, T_Var v_dst, T_Var v_addr, int i_offset = 0, bool gds = false, bool setM0 = false)
		{
			if (setM0 == true)
			{
				op2("s_mov_b32", "m0", -1);
			}

			size_t tmpIdx;
			std::string str = "";
			str.append("ds_read_b");

			str.append(d2s(num * 32));
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(v_dst, num));
			str.append(", ");
			str.append(getVarStr(v_addr));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			if (gds == true)
			{
				str.append(" ");
				str.append("gds");
			}

			wrLine(str);

			// error check
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("dest reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_addr.type != E_VarType::Vgpr)
			{
				str.append("ds addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		E_ReturnState ds_write_dword(int num, T_Var v_addr, T_Var v_dat, int i_offset = 0, bool gds = false, bool setM0 = false)
		{
			if (setM0 == true)
			{
				op2("s_mov_b32", "m0", -1);
			}

			size_t tmpIdx;
			std::string str = "";
			str.append("ds_write_b");

			str.append(d2s(num * 32));
			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(v_addr));
			str.append(", ");
			str.append(getVarStr(v_dat, num));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			if (gds == true)
			{
				str.append(" ");
				str.append("gds");
			}

			wrLine(str);
			
			// error check
			if (v_addr.type != E_VarType::Vgpr)
			{
				str.append("ds addr reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("data reg not vgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}

		/************************************************************************************/
		/* VALU																				*/
		/************************************************************************************/
		template <typename T>
		E_ReturnState v_add_u32(T_Var c, T a, T_Var b, std::string comment = "")
		{
			if (IsaArch == E_IsaArch::Gfx803)
			{
				op4("v_add_u32", c, "vcc", a, b);
			}
			else if (IsaArch >= E_IsaArch::Gfx900)
			{
				op3("v_add_u32", c, a, b, false, comment);
			}
			return E_ReturnState::SUCCESS;
		}
		template <typename T>
		E_ReturnState v_addc_u32(T_Var c, T a, T_Var b)
		{
			if (IsaArch == E_IsaArch::Gfx803)
			{
				op4("v_add_u32", c, "vcc", a, b);
			}
			else if (IsaArch >= E_IsaArch::Gfx900)
			{
				op4("v_add_co_u32", c, "vcc", a, b);
			}
			return E_ReturnState::SUCCESS;
		}
		template <typename T1, typename T2>
		E_ReturnState v_addc_co_u32(T_Var c, T1 a, T2 b)
		{
			if (IsaArch == E_IsaArch::Gfx803)
			{
				op5("v_addc_u32", c, "vcc", a, b, "vcc");
			}
			else if (IsaArch >= E_IsaArch::Gfx900)
			{
				op5("v_addc_co_u32", c, "vcc", a, b, "vcc");
			}
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState v_add3_u32(T_Var d, T_Var a, T_Var b, T_Var c)
		{
			if (IsaArch == E_IsaArch::Gfx803)
			{
				v_addc_u32(d, a, b);
				v_addc_co_u32(d, d, c);
			}
			else if (IsaArch >= E_IsaArch::Gfx900)
			{
				op4("v_add3_u32", d, a, b, c);
			}
			return E_ReturnState::SUCCESS;
		}
		
		template <typename T1, typename T2>
		E_ReturnState v_subb_u32(T_Var c, T1 a, T2 b)
		{
			if (IsaArch == E_IsaArch::Gfx803)
			{
				op4("v_sub_u32", c, "vcc", a, b);
			}
			else if (IsaArch >= E_IsaArch::Gfx900)
			{
				op4("v_sub_co_u32", c, "vcc", a, b);
			}
			return E_ReturnState::SUCCESS;
		}
		template <typename T1, typename T2>
		E_ReturnState v_subb_co_u32(T_Var c, T1 a, T2 b)
		{
			if (IsaArch == E_IsaArch::Gfx803)
			{
				op5("v_subb_u32", c, "vcc", a, b, "vcc");
			}
			else if (IsaArch >= E_IsaArch::Gfx900)
			{
				op5("v_subb_co_u32", c, "vcc", a, b, "vcc");
			}
			return E_ReturnState::SUCCESS;
		}

		/************************************************************************************/
		/* 通用操作																			*/
		/************************************************************************************/
		void op0(std::string op)
		{
			std::string str = "";
			str.append(op);

			wrLine(str);
		}
		template <typename T1>
		void op1(std::string op, T1 dst, int i_offset = 0)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			wrLine(str);
		}
		template <typename T1>
		void op1(std::string op, T1 dst, std::string flag)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));

			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			str.append(flag.c_str());

			wrLine(str);
		}
		template <typename T1, typename T2>
		void op2(std::string op, T1 dst, T2 src, int i_offset = 0)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			str.append(getVarStr(src));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			wrLine(str);
		}
		template <typename T1, typename T2>
		void op2(std::string op, T1 dst, T2 src, std::string flag)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			str.append(getVarStr(src));

			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			str.append(flag.c_str());

			wrLine(str);
		}
		template <typename T1, typename T2>
		void op2dpp(std::string op, T1 dst, T2 src0, std::string flag = "")
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			str.append(getVarStr(src0));

			str.append(" ");
			str.append(flag);

			wrLine(str);
		}
		
		template <typename T1, typename T2>
		void op2f(std::string op, T1 dst, T2 src, int i_offset = 0)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getfVarStr(dst));
			str.append(", ");
			str.append(getfVarStr(src));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			wrLine(str);
		}
		template <typename T1, typename T2>
		void op2f(std::string op, T1 dst, T2 src, std::string flag)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getfVarStr(dst));
			str.append(", ");
			str.append(getfVarStr(src));

			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			str.append(flag.c_str());

			wrLine(str);
		}
		
		template <typename T1, typename T2>
		void op2h(std::string op, T1 dst, T2 src, int i_offset = 0)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(gethVarStr(dst));
			str.append(", ");
			str.append(gethVarStr(src));

			if (i_offset != 0)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("offset:");
				str.append(getVarStr(i_offset));
			}

			wrLine(str);
		}
		template <typename T1, typename T2>
		void op2h(std::string op, T1 dst, T2 src, std::string flag)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(gethVarStr(dst));
			str.append(", ");
			str.append(gethVarStr(src));

			tmpIdx = FLAG_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			str.append(flag.c_str());

			wrLine(str);
		}

		template <typename T1, typename T2, typename T3>
		void op3(std::string op, T1 dst, T2 src0, T3 src1, bool glc = false, std::string comment = "")
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			str.append(getVarStr(src0));
			str.append(", ");
			str.append(getVarStr(src1));

			if (glc == true)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("glc");
			}
						
			if (comment != "")
			{
				tmpIdx = COMMON_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("// ");
				str.append(comment);
			}

			wrLine(str);
		}
		template <typename T1, typename T2, typename T3>
		void op3h(std::string op, T1 dst, T2 src0, T3 src1, bool glc = false, std::string comment = "")
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(gethVarStr(dst));
			str.append(", ");
			str.append(gethVarStr(src0));
			str.append(", ");
			str.append(gethVarStr(src1));

			if (glc == true)
			{
				tmpIdx = FLAG_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("glc");
			}

			if (comment != "")
			{
				tmpIdx = COMMON_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");
				str.append("// ");
				str.append(comment);
			}

			wrLine(str);
		}
		template <typename T1, typename T2, typename T3>
		void op3dpp(std::string op, T1 dst, T2 src0, T3 src1, std::string flag = "")
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			//str.append("abs(");	
			str.append(getVarStr(src0));
			//str.append(")");
			str.append(", ");
			//str.append("abs(");	
			str.append(getVarStr(src1));
			//str.append(") ");

			//str.append("quad_perm:[0, 0, 0, 0] wave_shr:1 bound_ctrl:0");
			//str.append("wave_shl:1 bound_ctrl:1");
			//str.append("wave_shr:1 row_mask:3 bound_ctrl:0");
			//str.append("wave_shr:1 bank_mask:3 bound_ctrl:0");
			//str.append("row_shl:4 bound_ctrl:0");
			//str.append("row_shl:4 row_mask:1 bound_ctrl:0");
			//str.append("row_shl:1 bank_mask:5 bound_ctrl:0");
			//str.append("row_bcast:15");
			str.append(" ");
			str.append(flag);

			wrLine(str);
		}
		template <typename T1, typename T2, typename T3, typename T4>
		void op4(std::string op, T1 dst, T2 src0, T3 src1, T4 src2, bool glc = false)
		{
			int tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			str.append(getVarStr(src0));
			str.append(", ");
			str.append(getVarStr(src1));
			str.append(", ");
			str.append(getVarStr(src2));

			tmpIdx = FLAG_START_COL - str.length();
			if (tmpIdx <= 0)tmpIdx = 1;
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");
			if (glc == true)
			{
				str.append("glc");
			}

			wrLine(str);
		}
		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		void op5(std::string op, T1 dst, T2 src0, T3 src1, T4 src2, T5 src3)
		{
			size_t tmpIdx;
			std::string str = "";
			str.append(op);

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append(getVarStr(dst));
			str.append(", ");
			str.append(getVarStr(src0));
			str.append(", ");
			str.append(getVarStr(src1));
			str.append(", ");
			str.append(getVarStr(src2));
			str.append(", ");
			str.append(getVarStr(src3));

			wrLine(str);
		}

		E_ReturnState s_wait_cnt0()
		{
			size_t tmpIdx;

			std::string str = "";
			str.append("s_waitcnt");

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append("0");

			wrLine(str);

			// error check
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState s_wait_lgkmcnt(unsigned int cnt)
		{
			size_t tmpIdx;

			std::string str = "";
			str.append("s_waitcnt");

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append("lgkmcnt");
			str.append("(" + d2s(cnt) + ")");

			wrLine(str);

			// error check
			str = "";
			if (!((cnt >= 0) && (cnt <= 15)))
			{
				str.append("lgkmcnt is over 4-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState s_wait_vmcnt(unsigned int cnt)
		{
			size_t tmpIdx;

			std::string str = "";
			str.append("s_waitcnt");

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append("vmcnt");
			str.append("(" + d2s(cnt) + ")");

			wrLine(str);

			// error check
			str = "";
			if (!((cnt >= 0) && (cnt <= 63)))
			{
				str.append("vmcnt is over 6-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState s_wait_lgkm_exp_cnt(unsigned int cnt)
		{
			size_t tmpIdx;

			std::string str = "";
			str.append("s_waitcnt");

			tmpIdx = PARAM_START_COL - str.length();
			for (int i = 0; i < tmpIdx; i++)
				str.append(" ");

			str.append("lgkmcnt");
			str.append("(" + d2s(cnt) + ")");
			str.append(" & ");
			str.append("expcnt");
			str.append("(" + d2s(cnt) + ")");

			wrLine(str);

			// error check
			str = "";
			if (!((cnt >= 0) && (cnt <= 15)))
			{
				str.append("lgkmcnt is over 4-bit");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			return E_ReturnState::SUCCESS;
		}
#pragma endregion

#pragma region CMB_OPT
		/************************************************************************************/
		/* 设置buffer寄存器																	*/
		/************************************************************************************/
		typedef enum desc_num_fmt_enum
		{
			num_fmt_unorm = 0,
			num_fmt_snorm = 1,
			num_fmt_uscaled = 2,
			num_fmt_sscaled = 3,
			num_fmt_uint = 4,
			num_fmt_sint = 5,
			num_fmt_reserved = 6,
			num_fmt_float = 7
		}e_desc_num_fmt;
		typedef enum desc_dat_fmt_enum
		{
			dat_fmt_invalid = 0,
			dat_fmt_8 = 1,
			dat_fmt_16 = 2,
			dat_fmt_8_8 = 3,
			dat_fmt_32 = 4,
			dat_fmt_16_16 = 5,
			dat_fmt_10_11_11 = 6,
			dat_fmt_11_11_10 = 7,
			dat_fmt_10_10_10_2 = 8,
			dat_fmt_2_10_10_10 = 9,
			dat_fmt_8_8_8_8 = 10,
			dat_fmt_32_32 = 11,
			dat_fmt_16_16_16_16 = 12,
			dat_fmt_32_32_32 = 13,
			dat_fmt_32_32_32_32 = 14,
			dat_fmt_reserved = 15
		}e_desc_dat_fmt;
		typedef enum desc_idx_stride_enum
		{
			idx_stride_8 = 0,
			idx_stride_16 = 1,
			idx_stride_32 = 2,
			idx_stride_64 = 3
		}e_desc_idx_stride;
		E_ReturnState f_set_buffer_desc(
			T_Var s_desc,
			T_Var s_base,
			unsigned int stride,
			unsigned int record_num,
			bool add_tid_en,
			e_desc_num_fmt num_fmt = e_desc_num_fmt::num_fmt_float,
			e_desc_dat_fmt dat_fmt = e_desc_dat_fmt::dat_fmt_32,
			bool swizzle_en = false,
			e_desc_idx_stride idx_stride = e_desc_idx_stride::idx_stride_8,
			bool cache_swizzle = false)
		{
			// desc0
			op2("s_mov_b64", s_desc ^ 2, s_base ^ 2);

			// desc1
			unsigned int dsc1_tmp = stride & 0x3FFF;
			dsc1_tmp = dsc1_tmp << 16;
			if (cache_swizzle == true)
			{
				dsc1_tmp |= (unsigned int)1 << 30;
			}
			if (swizzle_en == true)
			{
				dsc1_tmp |= (unsigned int)1 << 31;
			}
			op3("s_or_b32", s_desc + 1, s_desc + 1, dsc1_tmp);

			// desc2
			op2("s_mov_b32", s_desc + 2, record_num);

			// desc3
			unsigned int dsc3_tmp = ((unsigned int)num_fmt & 0x7) << 12;
			if (add_tid_en == true)
			{
				dsc3_tmp |= ((stride & 0x3C000) >> 14) << 15;
			}
			else
			{
				dsc3_tmp |= ((unsigned int)dat_fmt & 0xF) << 15;
			}
			if (swizzle_en == true)
			{
				dsc3_tmp |= (unsigned int)idx_stride << 21;
			}
			if (add_tid_en == true)
			{
				dsc3_tmp |= 1 << 23;
			}
			op2("s_mov_b32", s_desc + 3, dsc3_tmp);

			// error check
			std::string str = "";
			if (s_desc.type != E_VarType::Sgpr)
			{
				str.append("buffer descriptor not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_desc.gpr.len != 4)
			{
				str.append("buffer descriptor not 4-dword");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.type != E_VarType::Sgpr)
			{
				str.append("buffer obj base address not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}

		/************************************************************************************/
		/* 读取硬件寄存器																		*/
		/************************************************************************************/
		typedef enum hw_reg_enum
		{
			hw_reg_mode = 1,
			hw_reg_status = 2,
			hw_reg_hw_id = 4,
			hw_reg_gpr_alloc = 5,
			hw_reg_lds_alloc = 6,
			hw_reg_ib_sts = 7
		}e_hw_reg;
		E_ReturnState f_read_hw_reg(T_Var s_dst, e_hw_reg hwRegId)
		{
			unsigned int imm = 0;
			unsigned int size = 31;
			unsigned int offset = 0;

			imm = ((size & 0x1F) << 11) | ((offset & 0x0000001F) << 6) | ((unsigned int)hwRegId & 0x3F);
			op2("s_getreg_b32", s_dst, imm);

			// error check
			std::string str = "";
			if (s_dst.type != E_VarType::Sgpr)
			{
				str.append("dest reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
		template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
		E_ReturnState f_read_hw_reg_hw_id(
			T1 s_wave_id,
			T2 s_simd_id,
			T3 s_pipe_id,
			T4 s_cu_id,
			T5 s_sh_id,
			T6 s_se_id,
			T7 s_tg_id,
			T8 s_vm_id,
			T9 s_queue_id,
			T10 s_state_id,
			T11 me_id)
		{
			unsigned int src2;

			T_Var s_dest = newSgpr("s_dest");
			f_read_hw_reg(s_dest, hw_reg_hw_id);

			// wave id
			if (getVarType(s_wave_id) != E_VarType::Off)
			{
				src2 = ((4 & 0x7F) << 16) | ((0 & 0x1F) << 0);
				op3("s_bfe_u32", s_wave_id, s_dest, src2);
			}

			// simd id
			if (getVarType(s_simd_id) != E_VarType::Off)
			{
				src2 = ((2 & 0x7F) << 16) | ((4 & 0x1F) << 0);
				op3("s_bfe_u32", s_simd_id, s_dest, src2);
			}

			// pipe id
			if (getVarType(s_pipe_id) != E_VarType::Off)
			{
				src2 = ((2 & 0x7F) << 16) | ((6 & 0x1F) << 0);
				op3("s_bfe_u32", s_pipe_id, s_dest, src2);
			}

			// cu id
			if (getVarType(s_cu_id) != E_VarType::Off)
			{
				src2 = ((4 & 0x7F) << 16) | ((8 & 0x1F) << 0);
				op3("s_bfe_u32", s_cu_id, s_dest, src2);
			}

			// shader array id
			if (getVarType(s_sh_id) != E_VarType::Off)
			{
				src2 = ((1 & 0x7F) << 16) | ((12 & 0x1F) << 0);
				op3("s_bfe_u32", s_sh_id, s_dest, src2);
			}

			// shader engine id
			if (getVarType(s_se_id) != E_VarType::Off)
			{
				src2 = ((2 & 0x7F) << 16) | ((13 & 0x1F) << 0);
				op3("s_bfe_u32", s_se_id, s_dest, src2);
			}

			// thread group id
			if (getVarType(s_tg_id) != E_VarType::Off)
			{
				src2 = ((4 & 0x7F) << 16) | ((16 & 0x1F) << 0);
				op3("s_bfe_u32", s_tg_id, s_dest, src2);
			}

			// vm id
			if (getVarType(s_vm_id) != E_VarType::Off)
			{
				src2 = ((4 & 0x7F) << 16) | ((20 & 0x1F) << 0);
				op3("s_bfe_u32", s_vm_id, s_dest, src2);
			}

			// queue id
			if (getVarType(s_queue_id) != E_VarType::Off)
			{
				src2 = ((3 & 0x7F) << 16) | ((24 & 0x1F) << 0);
				op3("s_bfe_u32", s_queue_id, s_dest, src2);
			}

			// gfx context(state) id
			if (getVarType(s_state_id) != E_VarType::Off)
			{
				src2 = ((3 & 0x7F) << 16) | ((27 & 0x1F) << 0);
				op3("s_bfe_u32", s_state_id, s_dest, src2);
			}

			// microengine id
			if (getVarType(me_id) != E_VarType::Off)
			{
				src2 = ((2 & 0x7F) << 16) | ((30 & 0x1F) << 0);
				op3("s_bfe_u32", me_id, s_dest, src2);
			}

			delVar(s_dest);

			// error check
			std::string str = "";
			if ((getVarType(s_wave_id) != E_VarType::Sgpr) && (getVarType(s_wave_id) != E_VarType::Off))
			{
				str.append("wave_id reg not sgpr");
				wrLine(str);
				return E_ReturnState::RTN_ERR;
			}

			return E_ReturnState::SUCCESS;
		}
				
		template<typename T>
		T_Var f_s_loop(T loopCnt, std::string loopName)
		{
			T_Var s_cnt = newSgpr("s_cnt");
			op2("s_mov_b32", s_cnt, loopCnt);
			tableCnt = 0;
			wrLine(loopName + ":");
			tableCnt++;
			return s_cnt;
		}
		E_ReturnState f_e_loop(T_Var s_cnt, std::string loopName)
		{
			op3("s_sub_u32", s_cnt, s_cnt, 1);
			op2("s_cmpk_eq_i32", s_cnt, 0);
			T_Var t_lab = newLaber(loopName);
			op1("s_cbranch_scc0", t_lab);
			delVar(s_cnt);
		}

		T_Var f_s_if(T_Var var1, std::string op_str, T_Var var2, std::string jump_name)
		{
			T_Var var_rtn;
			var_rtn.type = E_VarType::None;
			E_OpType op = getOp(op_str);
			T_Var t_lab = newLaber(jump_name);

			if (var1.type == E_VarType::Sgpr)
			{
				switch (op)
				{
				case E_OpType::CMPEQ:op2("s_cmp_eq_u32", var1, var2); break;
				case E_OpType::CMPNE:op2("s_cmp_lg_u32", var1, var2); break;
				case E_OpType::CMPGE:op2("s_cmp_ge_u32", var1, var2); break;
				case E_OpType::CMPGT:op2("s_cmp_gt_u32", var1, var2); break;
				case E_OpType::CMPLE:op2("s_cmp_le_u32", var1, var2); break;
				case E_OpType::CMPLT:op2("s_cmp_lt_u32", var1, var2); break;
				}
				op1("s_cbranch_scc0", t_lab);
			}
			else if (var1.type == E_VarType::Vgpr)
			{
				T_Var v_exec_bck = newVgpr("v_exec_bck", 2, 2);
				op2("v_mov_b32", v_exec_bck[0], "exec_lo");
				op2("v_mov_b32", v_exec_bck[1], "exec_hi");
				switch (op)
				{
				case E_OpType::CMPEQ:op3("v_cmpx_eq_u32", "vcc", var1, var2); break;
				case E_OpType::CMPNE:op3("v_cmpx_lg_u32", "vcc", var1, var2); break;
				case E_OpType::CMPGE:op3("v_cmpx_ge_u32", "vcc", var1, var2); break;
				case E_OpType::CMPGT:op3("v_cmpx_gt_u32", "vcc", var1, var2); break;
				case E_OpType::CMPLE:op3("v_cmpx_le_u32", "vcc", var1, var2); break;
				case E_OpType::CMPLT:op3("v_cmpx_lt_u32", "vcc", var1, var2); break;
				}
				op1("s_cbranch_execz", t_lab);
				return v_exec_bck;
			}

			return var_rtn;
		}
		T_Var f_s_if(T_Var var1, std::string op_str, int var2, std::string jump_name)
		{
			T_Var var_rtn;
			var_rtn.type = E_VarType::None;
			E_OpType op = getOp(op_str);
			T_Var t_lab = newLaber(jump_name);

			if (var1.type == E_VarType::Sgpr)
			{
				switch (op)
				{
				case E_OpType::CMPEQ:op2("s_cmpk_eq_u32", var1, var2); break;
				case E_OpType::CMPNE:op2("s_cmpk_lg_u32", var1, var2); break;
				case E_OpType::CMPGE:op2("s_cmpk_ge_u32", var1, var2); break;
				case E_OpType::CMPGT:op2("s_cmpk_gt_u32", var1, var2); break;
				case E_OpType::CMPLE:op2("s_cmpk_le_u32", var1, var2); break;
				case E_OpType::CMPLT:op2("s_cmpk_lt_u32", var1, var2); break;
				}
				op1("s_cbranch_scc0", t_lab);
			}
			else if (var1.type == E_VarType::Vgpr)
			{
				T_Var v_tmp = newVgpr("v_tmp");
				T_Var v_exec_bck = newVgpr("v_exec_bck", 2, 2);
				op2("v_mov_b32", v_tmp, var2);
				op2("v_mov_b32", v_exec_bck[0], "exec_lo");
				op2("v_mov_b32", v_exec_bck[1], "exec_hi");
				switch (op)
				{
				case E_OpType::CMPEQ:op3("v_cmpx_eq_u32", "vcc", var1, v_tmp); break;
				case E_OpType::CMPNE:op3("v_cmpx_lg_u32", "vcc", var1, v_tmp); break;
				case E_OpType::CMPGE:op3("v_cmpx_ge_u32", "vcc", var1, v_tmp); break;
				case E_OpType::CMPGT:op3("v_cmpx_gt_u32", "vcc", var1, v_tmp); break;
				case E_OpType::CMPLE:op3("v_cmpx_le_u32", "vcc", var1, v_tmp); break;
				case E_OpType::CMPLT:op3("v_cmpx_lt_u32", "vcc", var1, v_tmp); break;
				}
				op1("s_cbranch_execz", t_lab);
				delVar(v_tmp);
				return v_exec_bck;
			}

			return var_rtn;
		}
		void f_e_if(T_Var var_del, std::string jump_name)
		{
			T_Var t_lab = newLaber(jump_name);
			wrLaber(t_lab);

			if (var_del.type != E_VarType::None)
			{
				delVar(var_del);
			}
		}

		T_Var f_record_time()
		{
			T_Var t = newSgpr("timer", 2, 2);
			op1("s_memtime", t ^ 2);
			s_wait_lgkmcnt(0);
			return t;
		}
		T_Var f_start_timer()
		{
			T_Var t0 = newSgpr("timer0", 2, 2);
			op1("s_memtime", t0 ^ 2);
			return t0;
		}
		T_Var f_stop_timer()
		{
			T_Var t1 = newSgpr("timer1", 2, 2);
			op1("s_memtime", t1 ^ 2);
			s_wait_lgkmcnt(0);
			return t1;
		}
		T_Var f_elapsed_time(T_Var t0, T_Var t1)
		{
			T_Var dt = newSgpr("diff_time", 2, 2);
			op3("s_sub_u32", dt[0], t1[0], t0[0]);
			op3("s_subb_u32", dt[1], t1[1], t0[1]);
			delVar(t0);
			delVar(t1);
			return dt;
		}
		T_Var f_elapsed_time(T_Var t0)
		{
			T_Var t1 = f_record_time();
			T_Var dt = newSgpr("diff_time", 2, 2);
			op3("s_sub_u32", dt[0], t1[0], t0[0]);
			op3("s_subb_u32", dt[1], t1[1], t0[1]);
			delVar(t0);
			delVar(t1);
			return dt;
		}

		template<typename T>
		E_ReturnState f_addr_add_byte(T_Var addr, T offset)
		{
			if (addr.type == E_VarType::Vgpr)
			{
				if (getVarType(offset) == E_VarType::Imm)
				{
					int imm_offset = getVarVal(offset);
					if ((imm_offset >= -16) && (imm_offset <= 64))
					{
						op4("v_add_co_u32", addr, "vcc", addr, offset);
						op5("v_addc_co_u32", addr + 1, "vcc", 0, addr + 1, "vcc");
					}
					else
					{
						T_Var t_offset = newVgpr("t_offset");
						op2("v_mov_b32", t_offset, offset);
						op4("v_add_co_u32", addr, "vcc", addr, offset);
						op5("v_addc_co_u32", addr + 1, "vcc", 0, addr + 1, "vcc");
						delVar(t_offset);
					}
				}
				else
				{
					op4("v_add_co_u32", addr, "vcc", addr, offset);
					op5("v_addc_co_u32", addr + 1, "vcc", 0, addr + 1, "vcc");
				}
			}
			else if (addr.type == E_VarType::Sgpr)
			{
				if (getVarType(offset) == E_VarType::Imm)
				{
					int imm_offset = getVarVal(offset);
					if ((imm_offset >= -16) && (imm_offset <= 64))
					{
						op3("s_add_u32", addr, addr, offset);
						op3("s_addc_u32", addr + 1, addr + 1, 0);
					}
					else
					{
						T_Var t_offset = newSgpr("t_offset");
						op2("s_mov_b32", t_offset, offset);
						op3("s_add_u32", addr, addr, t_offset);
						op3("s_addc_u32", addr + 1, addr + 1, 0);
					}
				}
				else
				{
					op3("s_add_u32", addr, addr, offset);
					op3("s_addc_u32", addr + 1, addr + 1, 0);
				}
			}
			else
			{
				return E_ReturnState::RTN_ERR;
			}
			return E_ReturnState::SUCCESS;
		}

		/************************************************************************************/
		/* 整型除法:                                                                       	*/
		/* a: 被除数                                                                       	*/
		/* b: 除数                                                                         	*/
		/* c: 商                                                                           	*/
		/* d: 余数                                                                         	*/
		/************************************************************************************/
		E_ReturnState fv_div_u32(T_Var a, T_Var b, T_Var c, T_Var d)
		{
			if (b.type == E_VarType::Imm || b.type == E_VarType::Macro)
			{
				if (isPow2(b.value) == true)
				{
					op3("v_lshrrev_b32", c, log2(b.value), a);
					op3("v_and_b32", d, modMask(b.value), a);
				}
				else
				{
					T_Var bb = newVgpr("b");
					op2("v_mov_b32", bb, b.value);

					op2("v_cvt_f32_u32", c, a);		// c = (float)a
					op2("v_mov_b32", d, 0.1);
					op3("v_add_f32", c, c, d);
					op2("v_cvt_f32_u32", d, bb);		// d = (float)b
					op2("v_rcp_f32", d, d);			// d = 1/(float)b
					op3("v_mul_f32", d, c, d);		// d = a/(float)b
					op2("v_cvt_flr_i32_f32", c, d);		// c = (int)(a/(float)b)
					op3("v_mul_u32_u24", d, c, bb);	// d = c * b
					if (IsaArch == E_IsaArch::Gfx900)
					{
						op3("v_sub_u32", d, a, d);		// d = a - c * b
					}
					else if (IsaArch == E_IsaArch::Gfx803)
					{
						op4("v_sub_u32", d, "vcc", a, d);		// d = a - c * b
					}
					delVar(bb);
				}
			}
			else
			{
				op2("v_cvt_f32_u32", c, a);		// c = (float)a
				op2("v_mov_b32", d, 0.1);
				op3("v_add_f32", c, c, d);
				op2("v_cvt_f32_u32", d, b);		// d = (float)b
				op2("v_rcp_f32", d, d);			// d = 1/(float)b
				op3("v_mul_f32", d, c, d);		// d = a/(float)b
				op2("v_cvt_flr_i32_f32", c, d);		// c = (int)(a/(float)b)
				op3("v_mul_u32_u24", d, c, b);	// d = c * b
				if (IsaArch == E_IsaArch::Gfx900)
				{
					op3("v_sub_u32", d, a, d);		// d = a - c * b
				}
				else if (IsaArch == E_IsaArch::Gfx803)
				{
					op4("v_sub_u32", d, "vcc", a, d);		// d = a - c * b
				}
			}
			return E_ReturnState::SUCCESS;
		}

		E_ReturnState fv_div_u32(T_Var a, int v, T_Var c, T_Var d)
		{
			if (isPow2(v) == true)
			{
				op3("v_lshrrev_b32", c, log2(v), a);
				op3("v_and_b32", d, modMask(v), a);
			}
			else
			{
				T_Var b = newVgpr("b");
				op2("v_mov_b32", b, v);

				op2("v_cvt_f32_u32", c, a);		// c = (float)a
				op2("v_mov_b32", d, 0.1);
				op3("v_add_f32", c, c, d);
				op2("v_cvt_f32_u32", d, b);		// d = (float)b
				op2("v_rcp_f32", d, d);			// d = 1/(float)b
				op3("v_mul_f32", d, c, d);		// d = a/(float)b
				op2("v_cvt_flr_i32_f32", c, d);		// c = (int)(a/(float)b)
				op3("v_mul_u32_u24", d, c, b);	// d = c * b
				if (IsaArch == E_IsaArch::Gfx900)
				{
					op3("v_sub_u32", d, a, d);		// d = a - c * b
				}
				else if (IsaArch == E_IsaArch::Gfx803)
				{
					op4("v_sub_u32", d, "vcc", a, d);		// d = a - c * b
				}
				delVar(b);
			}
		}
#pragma endregion

#pragma region GAS_REGION
		/************************************************************************/
		/* if...else...															*/
		/************************************************************************/
		void sIF(std::string param1, std::string op, std::string param2)
		{
			std::string str = sblk();
			str.append(".if (");
			str.append(param1);
			str.append(" " + op + " ");
			str.append(param2);
			str.append(")");

			indent();
			str.append("\n");
			kernelString.append(str);

		}
		void sELSE()
		{
			backSpace();
			std::string str = sblk();
			str.append(".else");
			indent();
			str.append("\n");
			kernelString.append(str);
		}
		void eIF()
		{
			backSpace();
			std::string str = sblk();
			str.append(".endif");
			str.append("\n");
			kernelString.append(str);
		}

		/************************************************************************/
		/* for 循环																*/
		/************************************************************************/
		void sFOR(int loop)
		{
			std::string str = sblk();
			str.append(".rept(");
			str.append(d2s(loop));
			str.append(")");
			indent();
			str.append("\n");
			kernelString.append(str);
		}
		void eFOR()
		{
			backSpace();
			std::string str = sblk();
			str.append(".endr");
			str.append("\n");
			kernelString.append(str);
		}

		/************************************************************************/
		/* 定义函数																*/
		/************************************************************************/
		void sFUNC(std::string name, int parCnt, ...)
		{
			size_t tmpIdx;
			std::string str = sblk();
			str.append(".macro ");
			str.append(name);

			if (parCnt > 0)
			{
				tmpIdx = PARAM_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");

				va_list args;
				char * arg;
				va_start(args, parCnt);

				for (int i = 0; i < parCnt - 1; i++)
				{
					arg = va_arg(args, char*);
					str.append(arg);
					str.append(", ");
				}
				arg = va_arg(args, char*);
				str.append(arg);

				va_end(args);
			}

			indent();
			str.append("\n");
			kernelString.append(str);
		}
		void eFUNC()
		{
			backSpace();
			std::string str = sblk();
			str.append(".endm\n");
			str.append("\n");
			kernelString.append(str);
		}
		void FUNC(std::string func, int parCnt, ...)
		{
			size_t tmpIdx;

			std::string str = sblk();
			str.append(func);

			if (parCnt > 0)
			{
				tmpIdx = PARAM_START_COL - str.length();
				for (int i = 0; i < tmpIdx; i++)
					str.append(" ");

				va_list args;
				char * arg;
				va_start(args, parCnt);

				for (int i = 0; i < parCnt - 1; i++)
				{
					arg = va_arg(args, char*);
					str.append(arg);
					str.append(", ");
				}
				arg = va_arg(args, char*);
				str.append(arg);

				va_end(args);
			}

			str.append("\n");
			kernelString.append(str);
		}

		/************************************************************************/
		/* 变量设置																*/
		/************************************************************************/
		void refGPR(std::string tgpr, std::string gpr)
		{
			std::string str = sblk();
			str.append(tgpr);
			str.append(" = \\");
			str.append(gpr);
			str.append("\n");
			kernelString.append(str);
		}
		void setGPR(std::string tgpr, int idx)
		{
			std::string str = sblk();
			str.append(tgpr);
			str.append(" = ");
			str.append(d2s(idx));
			str.append("\n");
			kernelString.append(str);
		}
#pragma endregion
	};
}
