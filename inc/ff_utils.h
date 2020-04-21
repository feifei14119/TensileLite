#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // linux: C style memset
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <string>
#include <memory.h>
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#pragma comment(lib,"Winmm.lib")	//For timeSetEvent
#else
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996) 
//#pragma warning(disable: 2036) 
#endif

namespace feifei
{
	/************************************************************************/
	/* basic type define													*/
	/************************************************************************/
#pragma region BASIC

	typedef enum class ReturnStateEnum
	{
		SUCCESS = 0,
		RTN_WARN = 1,	// 警告,继续执行
		RTN_ERR = 2,	// 错误, 抛出异常并退出此函数
		RTN_FATAL = 3	// 失败, 终止程序
	} E_ReturnState;
	typedef enum DataTypeEnum
	{
		Float,
		Int,
		Fp32,
		Fp64,
		Fp16,
		Bf16,
		String,
		UInt8,
		Int8,
		Uint16,
		Int16,
		Int32
	} E_DataType;
#define ChkErr(val) do{\
	E_ReturnState val_hold = val; \
	if(val_hold == E_ReturnState::RTN_ERR) return val;\
	if(val_hold == E_ReturnState::RTN_FATAL) exit(EXIT_FAILURE);\
}while(0)

#pragma endregion

	/************************************************************************/
	/* arithmetic															*/
	/************************************************************************/
#pragma region ARITH

#define	MIN_FP32_ERR		(1e-6)
	
	inline int log2Int(int value)
	{
		int log2 = 0;
		while (value > 1) { value = value / 2; log2++; }
		return log2;
	}
	inline unsigned int f32_as_u32(float f) { union { float f; unsigned int u; } v; v.f = f; return v.u; }
	inline float u32_as_f32(unsigned int u) { union { float f; unsigned int u; } v; v.u = u; return v.f; }
	inline int clamp_int(int i, int l, int h) { return std::min(std::max(i, l), h); }
	inline short cvtFP32toFP16(float f)
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
	inline float cvtFP16toFP32(unsigned short a)
	{
		unsigned int u = ((a << 13) + 0x70000000U) & 0x8fffe000U;
		unsigned int v = f32_as_u32(u32_as_f32(u) * pow(1.0, 112)) + 0x38000000U;
		u = (a & 0x7fff) != 0 ? v : u;
		return u32_as_f32(u) * pow(1.0, -112);
	}
	inline short cvtFP32toBF16(float in)
	{
		return (*(uint32_t*)(&in) >> 16);
	}
	inline float cvtBF16toFP32(unsigned short in)
	{
		uint32_t tmp = in << 16;
		return *(float*)(&tmp);
	}

#pragma endregion

	/************************************************************************/
	/* file and log															*/
	/************************************************************************/
#pragma region LOG

#ifdef _WIN32
#define	DIR_SPT ('\\')
#else
#define	DIR_SPT ('/')
#endif

#define INFO(fmt,...)		PrintInfo(fmt,##__VA_ARGS__)
#define	LOG(fmt,...)		PrintLog(fmt,##__VA_ARGS__)
#define WARN(fmt,...)		PrintWarning(__FILE__,__LINE__,fmt,##__VA_ARGS__)
#define ERR(fmt,...)		do{PrintError(__FILE__,__LINE__,fmt,##__VA_ARGS__);return E_ReturnState::RTN_ERR;}while(0)
#define FATAL(fmt,...)		PrintFatal(__FILE__,__LINE__,fmt,##__VA_ARGS__)
	
	typedef enum class FormatEnum
	{
		Nomal = 1,
		Hex = 2,
		Bin = 3
	}E_DataFormat;

	std::string fmtTime(double sec);
	std::string fmtFreq(uint64_t freqHz);
	std::string fmtSize(size_t szByte);

	// 输出分隔符
	void PrintSeperator(const char c, std::ostream *sm = &std::cout);
	// cout输出
	void PrintInfo(const char * format, ...);
	void PrintInfo(std::string msg, ...);
	// 带时间戳的clog输出
	void PrintLog(const char * format, ...);
	void PrintLog(std::string msg, ...);
	// 带时间戳和错误位置的clog输出
	void PrintWarning(const char *file, int line, const char * format, ...);
	void PrintWarning(const char *file, int line, std::string msg, ...);
	// 带时间戳和错误位置并返回错误的cerr输出
	E_ReturnState PrintError(const char *file, int line, const char * format, ...);
	E_ReturnState PrintError(const char *file, int line, std::string msg, ...);
	// 带时间戳和错误位置并终止程序的cerr输出
	void PrintFatal(const char *file, int line, const char * format, ...);
	void PrintFatal(const char *file, int line, std::string msg, ...);
		
	template<typename DataType>
		void log_data_mem(void * addr, uint64_t len, std::string name = "",
			E_DataFormat dataFmt = E_DataFormat::Nomal, int fmtLen = 1,
			uint64_t startIdx = 0, uint64_t endIdx = 0, int numPerRow = 8, std::vector<uint64_t> * dim_size = 0, FILE * ostream = stdout)
	{
		if (ostream == stdout)
		{
			PrintSeperator('-');
			printf(" data_name : %s", name.data());
			if (dim_size != 0)
			{
				printf(": [");
				for (int i = 0; i < dim_size->size() - 1; i++)
				{
					printf("%lld, ", (*dim_size)[i]);
				}
				printf("%lld]", (*dim_size)[dim_size->size() - 1]);
			}
			printf("\n");
			PrintSeperator('-');
		}
		else
		{
			fprintf(ostream, "data_name: %s\n", name.data());
		}

		if (endIdx == 0)
			endIdx = len - 1;
		if (endIdx >= len)
			endIdx = len - 1;

		std::string fmtstr;
		if (dataFmt == E_DataFormat::Bin)
		{
			int bn = sizeof(DataType);
			fmtstr = "%0" + std::to_string(bn * 8) + "B";
		}
		else if (dataFmt == E_DataFormat::Hex)
		{
			int bn = sizeof(DataType);
			fmtstr = "0x%0" + std::to_string(bn * 2) + "X";
		}
		else
		{
			if (std::is_same<DataType, uint8_t>{} || std::is_same<DataType, int8_t>{} ||
				std::is_same<DataType, uint16_t>{} || std::is_same<DataType, int16_t>{} ||
				std::is_same<DataType, uint32_t>{} || std::is_same<DataType, int32_t>{})
				fmtstr = (fmtLen == 0) ? "%d" : "%0" + std::to_string(fmtLen) + "d";
			else if (std::is_same<DataType, uint64_t>{} || std::is_same<DataType, int64_t>{})
				fmtstr = (fmtLen == 0) ? "%lld" : "%0" + std::to_string(fmtLen) + "lld";
			else if (std::is_same<DataType, float>{} || std::is_same < DataType, double>{})
				fmtstr = (fmtLen == 0) ? "%.2f" : "%." + std::to_string(fmtLen) + "f";
			//else if (std::is_same<DataType, cplx_fp32>{} || std::is_same < DataType, cmpx_fp64>{})
				//fmtstr = (fmtLen == 0) ? "%.2f" : "%." + std::to_string(fmtLen) + "f";
			else
				fmtstr = "%f";
		}
		//if (!(std::is_same<DataType, cplx_fp32>{} || std::is_same < DataType, cmpx_fp64>{}))
		fmtstr += ", ";

		int i = 0;
		for (uint64_t idx = startIdx; idx <= endIdx; idx++, i++)
		{
			if (i % numPerRow == 0)
			{
				if (i != 0) fprintf(ostream, "\n");
				fprintf(ostream, "[%04d~%04d]: ", (int)idx, (int)(idx + numPerRow - 1));
			}

			//if (std::is_same<DataType, cplx_fp32>{}) {}
			//else if (std::is_same < DataType, cmpx_fp64>{}) {}
			//else
			{
				if (dataFmt == E_DataFormat::Hex)
				{
					fprintf(ostream, fmtstr.data(), ((uint32_t *)addr)[idx]);
				}
				else
				{
					fprintf(ostream, fmtstr.data(), ((DataType *)addr)[idx]);
				}
			}
		}

		fprintf(ostream, "\n");
	}
	
	extern void exec_cmd(std::string cmd);

	extern std::string get_curr_path();
	extern std::string get_work_path();
	extern std::string get_data_path();
	extern void init_work_path();
	extern void set_work_path(std::string path);
	extern void ensure_dir(const char * dir);

	extern std::string get_file_path(std::string fileName);
	extern std::string get_file_name(std::string fileName);

	extern E_ReturnState dump2_txt_file(std::string file_name, std::string str);
	
#pragma endregion

	/************************************************************************/
	/* timer																*/
	/************************************************************************/
#define ffSleepMS(t)	usleep((unsigned int)(t * 1000));

	/************************************************************************/
	/* data base															*/
	/************************************************************************/

	/************************************************************************/
	/* command line parameter												*/
	/************************************************************************/
#pragma region CMD_IF

	/* NEED TODO: */
	typedef enum ArgIdEnum
	{
		CMD_ARG_HELP,
		GEMM_ARG_TYPE,
		GEMM_ARG_M,
		GEMM_ARG_N,
		GEMM_ARG_K,
		GEMM_ARG_PAD,
		GEMM_ARG_MT0,
		GEMM_ARG_MT1,
		GEMM_ARG_WT0,
		GEMM_ARG_WT1,
		GEMM_ARG_DU,
		GEMM_ARG_MFMA_MN,
		GEMM_ARG_VERIFY,
		GEMM_ARG_LOOP,
		GEMM_ARG_BUFFER,
		GEMM_ARG_TENSILE
	} E_ArgId;

	typedef struct CmdArgType
	{
		E_ArgId id;
		std::string longName;
		char shortName;
		std::string value;
		int iValue;
		double fValue;
		std::string sValue;
		E_DataType type;
		std::string helpText;
	} T_CmdArg;

	class CmdArgs
	{
	public:
		CmdArgs();
		CmdArgs(int argc, char *argv[]);
		void * GetOneArg(E_ArgId id);
		static CmdArgs * GetCmdArgs();
		std::string ExecutePath() { return executePath; }

	private:
		static CmdArgs * pCmdArgs;
		int argsNum = 0;
		std::map<E_ArgId, T_CmdArg*> * argsMap;
		std::string executePath;

		/* NEED TODO: */
		void initCmdArgs()
		{
			addOneArg(CMD_ARG_HELP, E_DataType::String, "help", 'h', "help", "help infomation");
			addOneArg(GEMM_ARG_TYPE, E_DataType::Int, "1", 'd', "data-type", "data type. (1)");
			addOneArg(GEMM_ARG_M, E_DataType::Int, "1024", 'm', "gemm-m", "gemm m dim. (1024)");
			addOneArg(GEMM_ARG_N, E_DataType::Int, "1024", 'n', "gemm-n", "gemm n dim. (1024)");
			addOneArg(GEMM_ARG_K, E_DataType::Int, "1024", 'k', "gemm-k", "gemm k dim. (1024)");
			addOneArg(GEMM_ARG_PAD, E_DataType::Int, "64", 'p', "padding", "elements padding for m/n/k. (64)");
			addOneArg(GEMM_ARG_MT0, E_DataType::Int, "1", 'r', "mfma-pttn0", "mfma times in m dim. (1)");
			addOneArg(GEMM_ARG_MT1, E_DataType::Int, "1", 's', "mfma-pttn1", "mfma times in n dim. (1)");
			addOneArg(GEMM_ARG_WT0, E_DataType::Int, "1", 'x', "wave-pttn0", "wave number in m dim. (1)");
			addOneArg(GEMM_ARG_WT1, E_DataType::Int, "1", 'y', "wave-pttn1", "wave number in n dim. (1)");
			addOneArg(GEMM_ARG_MFMA_MN, E_DataType::Int, "32", 'z', "mfma-mn", "mfma instruction m/n width. (32)");
			addOneArg(GEMM_ARG_DU, E_DataType::Int, "16", 'u', "depth-u", "k dim loop unroll. (16)");
			addOneArg(GEMM_ARG_VERIFY, E_DataType::Int, "1", 'v', "verify", "enable cpu verify. (1)");
			addOneArg(GEMM_ARG_LOOP, E_DataType::Int, "100", 'l', "loop", "loop times for perf test. (100)");
			addOneArg(GEMM_ARG_BUFFER, E_DataType::Int, "3", 'f', "buffer", "lds buffer number. (3)");
			addOneArg(GEMM_ARG_TENSILE, E_DataType::Int, "0", 't', "tensile", "if use tensile format. (0)");
		}
		void helpText();
		void addOneArg(E_ArgId id, E_DataType dType, std::string defaultVal, char sName = '\0', std::string lName = "", std::string tHelp = "");
		void paserCmdArgs(int argc, char *argv[]);
		E_ReturnState setOneArg(char sName, std::string value);
		E_ReturnState setOneArg(std::string lName, std::string value);
		E_ReturnState setOneArgValue(T_CmdArg * arg, std::string value);
		void * getOneArgValue(T_CmdArg * arg);
	};

#pragma endregion

	/************************************************************************/
	/* task pool															*/
	/************************************************************************/

}
