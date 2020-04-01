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
			createStatus = E_ReturnState::SUCCESS;
		}
		std::string * GetKernelString()
		{
			return &kernelString;
		}

	protected:
		E_IsaArch IsaArch = E_IsaArch::Gfx900;
		std::string kernelString;
		E_ReturnState createStatus;

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
			{
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			ldsByteCount += groupLdsByte;
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState ldsAllocDword(size_t groupLdsDword)
		{
			size_t lds_size_byte = groupLdsDword * 4;
			if (ldsByteCount + lds_size_byte > MAX_LDS_SIZE)
			{
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_base.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.type != E_VarType::Sgpr)
			{
				str.append("base addr reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (getVarType(offset) == E_VarType::Vgpr)
			{
				str.append("offset reg are vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((idx_en == true) || (off_en == true))
			{
				if (getVarType(v_offset_idx) != E_VarType::Vgpr)
				{
					str.append("offset and index reg not vgpr");
					wrLine(str);
					createStatus = E_ReturnState::RTN_ERR;
					return E_ReturnState::RTN_ERR;
				}
			}
			if (s_desc.type != E_VarType::Sgpr)
			{
				str.append("buffer descriptor reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_desc.gpr.len != 4)
			{
				str.append("buffer descriptor reg not 4-dword");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			/*if (s_base_offset.type != E_VarType::Sgpr)
			{
				str.append("buffer obj base offset addr reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}*/
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset is not 12-bit unsigned int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((num > 1) && (lds_en == true))
			{
				str.append("lds direct only support 1dword");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((idx_en == true) || (off_en == true))
			{
				if (getVarType(v_offset_idx) != E_VarType::Vgpr)
				{
					str.append("offset and index reg not vgpr");
					wrLine(str);
					createStatus = E_ReturnState::RTN_ERR;
					return E_ReturnState::RTN_ERR;
				}
			}
			if (s_desc.type != E_VarType::Sgpr)
			{
				str.append("buffer descriptor reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (s_desc.gpr.len != 4)
			{
				str.append("buffer descriptor reg not 4-dword");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			/*if (s_base_offset.type != E_VarType::Sgpr)
			{
				str.append("buffer obj base offset addr reg not sgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}*/
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset is not 12-bit unsigned int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2)&&(is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= 0) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_offset_addr.type != E_VarType::Vgpr)
			{
				str.append("base addr reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2) && (is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2) && (is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_addr.gpr.len != 2)
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((v_offset_addr.gpr.len != 2) && (is64AddrMode == true))
			{
				str.append("base addr reg not 64-bit for 64-bit addr mode");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dst.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("store data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if ((getVarType(s_addr) != E_VarType::Sgpr) && (getVarType(s_addr) != E_VarType::Off))
			{
				str.append("offset reg not sgpr nor off");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (!((i_offset >= -4096) && (i_offset <= 4095)))
			{
				str.append("imm_offset over 13-bit int");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_addr.type != E_VarType::Vgpr)
			{
				str.append("ds addr reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			if (v_dat.type != E_VarType::Vgpr)
			{
				str.append("data reg not vgpr");
				wrLine(str);
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
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
				createStatus = E_ReturnState::RTN_ERR;
				return E_ReturnState::RTN_ERR;
			}
			return E_ReturnState::SUCCESS;
		}
#pragma endregion

		template<typename T>
		T_Var f_s_loop(T loopCnt, std::string loopName)
		{
			T_Var s_cnt = newSgpr("s_cnt");
			T_Var t_lab = newLaber(loopName);
			op3("s_sub_u32", s_cnt, loopCnt, 1);
			wrLaber(t_lab);
			return s_cnt;
		}
		E_ReturnState f_e_loop(T_Var s_cnt, std::string loopName)
		{
			op3("s_sub_u32", s_cnt, s_cnt, 1);
			T_Var t_lab = newLaber(loopName);
			op1("s_cbranch_scc0", t_lab);
			delVar(s_cnt);
		}
	};
}
