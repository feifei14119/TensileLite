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

	typedef void(*PVoidFunc)();
	typedef E_ReturnState(*PRetFunc)();
	typedef E_ReturnState(*PRetFunc2)(void* param);

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
#define MAX_16BIT_UINT		(65535)
#define PI					(3.1415926535897932384626433832795028841971)		// 定义圆周率值
#define PI_FP32				(3.1415926535897932384626433832795028841971f)		// 定义圆周率值

	inline int next2pow(int n)
	{
		int base = 1;
		for (int i = 0; i < 32; i++)
		{
			base = 1 << i;

			if (n <= base)
				break;
		}
		return base;
	}
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

#define isPow2(value)  ((value & (value - 1)) == 0)
#define modMask(value) (value - 1)
#define divCeil(a,b)	((a + b - 1) / b)

#define randInt10(a,b) ((rand() % (b-a)) + a)   	// [a,b)
#define randInt11(a,b) ((rand() % (b-a+1)) + a) 	// [a,b]
#define randInt01(a,b) ((rand() % (b-a)) + a + 1)	// (a,b]

	typedef struct cplx_fp32
	{
		float real, imag;

		cplx_fp32()
		{
			real = 0;
			imag = 0;
		}
		cplx_fp32(float d)
		{
			real = d;
			imag = d;
		}
		cplx_fp32(float r, float i)
		{
			real = r;
			imag = i;
		}

		cplx_fp32 operator + (cplx_fp32 b)
		{
			cplx_fp32 c;
			c.real = real + b.real;
			c.imag = imag + b.imag;
			return c;
		}
		void operator += (cplx_fp32 b)
		{
			real = real + b.real;
			imag = imag + b.imag;
		}
		cplx_fp32 operator - (cplx_fp32 b)
		{
			cplx_fp32 c;
			c.real = real - b.real;
			c.imag = imag - b.imag;
			return c;
		}
		void operator -= (cplx_fp32 b)
		{
			real = real - b.real;
			imag = imag - b.imag;
		}
		void operator = (cplx_fp32 b)
		{
			real = b.real;
			imag = b.imag;
		}
		void operator = (float b)
		{
			real = b;
			imag = b;
		}
		cplx_fp32 operator * (cplx_fp32 b)
		{
			cplx_fp32 c;
			c.real = real * b.real - imag * b.imag;
			c.imag = real * b.imag + imag * b.real;
			return c;
		}
		cplx_fp32 operator * (float b)
		{
			cplx_fp32 c;
			c.real = real * b;
			c.imag = imag * b;
			return c;
		}

		float abs()
		{
			return (float)sqrt(real * real + imag * imag);
		}
		cplx_fp32 conj()
		{
			cplx_fp32 c;
			c.real = real;
			c.imag = -imag;
			return c;
		}
	} cp_f;
	typedef struct cmpx_fp64
	{
		double real, imag;

		cmpx_fp64()
		{
			real = 0;
			imag = 0;
		}
		cmpx_fp64(double r, double i)
		{
			real = r;
			imag = i;
		}

		cmpx_fp64 operator + (cmpx_fp64 b)
		{
			cmpx_fp64 c;
			c.real = real + b.real;
			c.imag = imag + b.imag;
			return c;
		}
		cmpx_fp64 operator + (cplx_fp32 b)
		{
			cmpx_fp64 c;
			c.real = real + b.real;
			c.imag = imag + b.imag;
			return c;
		}
		void operator += (cmpx_fp64 b)
		{
			real = real + b.real;
			imag = imag + b.imag;
		}
		void operator += (cplx_fp32 b)
		{
			real = real + b.real;
			imag = imag + b.imag;
		}
		cmpx_fp64 operator - (cmpx_fp64 b)
		{
			cmpx_fp64 c;
			c.real = real - b.real;
			c.imag = imag - b.imag;
			return c;
		}
		cmpx_fp64 operator - (cplx_fp32 b)
		{
			cmpx_fp64 c;
			c.real = real - b.real;
			c.imag = imag - b.imag;
			return c;
		}
		void operator -= (cmpx_fp64 b)
		{
			real = real - b.real;
			imag = imag - b.imag;
		}
		void operator -= (cplx_fp32 b)
		{
			real = real - b.real;
			imag = imag - b.imag;
		}
		void operator = (cmpx_fp64 b)
		{
			real = b.real;
			imag = b.imag;
		}
		void operator = (cplx_fp32 b)
		{
			real = b.real;
			imag = b.imag;
		}
		cmpx_fp64 operator * (cmpx_fp64 b)
		{
			cmpx_fp64 c;
			c.real = real * b.real - imag * b.imag;
			c.imag = real * b.imag + imag * b.real;
			return c;
		}
		cmpx_fp64 operator * (cplx_fp32 b)
		{
			cmpx_fp64 c;
			c.real = real * b.real - imag * b.imag;
			c.imag = real * b.imag + imag * b.real;
			return c;
		}

		float abs()
		{
			return (float)sqrt(real * real + imag * imag);
		}
		cmpx_fp64 conj()
		{
			cmpx_fp64 c;
			c.real = real;
			c.imag = -imag;
			return c;
		}
	} cp_d;

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

	class LogFile
	{
	public:
		LogFile(std::string file_name, bool isNewFile = true);
		~LogFile();
		void Log(const char * format, ...);
		void Log(std::string msg, ...);

	protected:
		void ensureLogDir();
		std::string log_dir;
		std::string file_name;
		std::ofstream * fstream;
		char * PrintBuffer;
	}; 
	
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
			else if (std::is_same<DataType, cplx_fp32>{} || std::is_same < DataType, cmpx_fp64>{})
				fmtstr = (fmtLen == 0) ? "%.2f" : "%." + std::to_string(fmtLen) + "f";
			else
				fmtstr = "%f";
		}
		if (!(std::is_same<DataType, cplx_fp32>{} || std::is_same < DataType, cmpx_fp64>{}))
			fmtstr += ", ";

		int i = 0;
		for (uint64_t idx = startIdx; idx <= endIdx; idx++, i++)
		{
			if (i % numPerRow == 0)
			{
				if (i != 0) fprintf(ostream, "\n");
				fprintf(ostream, "[%04d~%04d]: ", (int)idx, (int)(idx + numPerRow - 1));
			}

			if (std::is_same<DataType, cplx_fp32>{})
			{
				fprintf(ostream, "<");
				fprintf(ostream, fmtstr.data(), ((float *)addr)[idx * 2 + 0]);
				fprintf(ostream, ",");
				fprintf(ostream, fmtstr.data(), ((float *)addr)[idx * 2 + 1]);
				fprintf(ostream, ">,");
			}
			else if (std::is_same < DataType, cmpx_fp64>{})
			{
				fprintf(ostream, "<");
				fprintf(ostream, fmtstr.data(), ((double *)addr)[idx * 2 + 0]);
				fprintf(ostream, ",");
				fprintf(ostream, fmtstr.data(), ((double *)addr)[idx * 2 + 1]);
				fprintf(ostream, ">,");
			}
			else
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

	extern size_t read_bin_file(std::string file_name, char * binary);
	extern E_ReturnState dump2_bin_file(std::string file_name, std::vector<char> *binary);
	extern E_ReturnState dump2_txt_file(std::string file_name, std::string str);
	
	template<typename DataType>
	uint64_t load_bin_mem(void ** addr, std::string file_name, uint64_t len = 0)
	{
		std::string file_name_full;
		file_name_full = get_data_path() + file_name;

		std::ifstream fin(file_name_full.c_str(), std::ios::in | std::ios::binary);
		if (!fin.is_open())
		{
			WARN("can't open bin file: " + file_name);
			return 0;
		}

		size_t binSize;
		fin.seekg(0, std::ios::end);
		binSize = (size_t)fin.tellg();

		size_t memSize;
		memSize = len * sizeof(DataType);

		if (memSize == 0)
		{
			memSize = binSize;
			len = memSize / sizeof(DataType);
		}

		if (*addr == NULL)
			*addr = malloc(memSize);

		fin.seekg(0, std::ios::beg);
		fin.read((char*)(*addr), memSize);

		fin.close();
		return len;
	}
	template<typename DataType>
	E_ReturnState dump_bin_mem(void * addr, std::string data_name, uint64_t len)
	{
		std::string file_name;
		file_name = get_data_path() + data_name + ".bin";

		std::ofstream fout(file_name.c_str(), std::ios::out | std::ios::binary);
		if (!fout.is_open())
		{
			ERR("can't open save file: " + file_name);
		}
		fout.write((char*)addr, len * sizeof(DataType));
		fout.close();
		return E_ReturnState::SUCCESS;
	}
	template<typename DataType>
	E_ReturnState dump_data_mem(void * addr, uint64_t len, std::string data_name = "",
		E_DataFormat dataFmt = E_DataFormat::Nomal, int fmtLen = 0,
		uint64_t startIdx = 0, uint64_t endIdx = 0, int numPerRow = 8, std::vector<uint64_t> * dim_size = 0)
	{
		std::string file_name;
		file_name = get_data_path() + data_name + ".txt";

		FILE *fp;
#ifdef _WIN32
		errno_t err = fopen_s(&fp, file_name.data(), "w");
		if (err != 0)
			return E_ReturnState::RTN_ERR;
#else
		fp = fopen(file_name.data(), "w");
		if (!fp)
			return E_ReturnState::RTN_ERR;;
#endif

		log_data_mem<DataType>(addr, len, data_name, dataFmt, fmtLen, startIdx, endIdx, numPerRow, dim_size, fp);

		fclose(fp);
		return E_ReturnState::SUCCESS;
	}

#pragma endregion

	/************************************************************************/
	/* timer																*/
	/************************************************************************/
#pragma region TIMER

#ifdef _WIN32 
#define ffSleepSec(t)	WinTimer::SleepSec(t)
#define ffSleepMS(t)	WinTimer::SleepMilliSec(t)
#else
#define ffSleepSec(t)	UnixTimer::SleepSec(t)
#define ffSleepMS(t)	UnixTimer::SleepMilliSec(t)
#endif
	class TimerBase
	{
	protected:
		timespec startTime;
		timespec stopTime;

	public:
		virtual void Restart() = 0;
		virtual void Stop() = 0;

		double ElapsedMilliSec = 0;
		double ElapsedNanoSec = 0;
	};

#ifdef _WIN32
	class WinTimer :public TimerBase
	{
	public:
		void Restart();
		void Stop();
		static void SleepSec(int sec);
		static void SleepSec(double sec);
		static void SleepMilliSec(int ms);
		static void SleepMilliSec(double ms);

	protected:
		LARGE_INTEGER cpuFreqHz;
		LARGE_INTEGER startCnt;
		LARGE_INTEGER stopCnt;
	};
#else
	class UnixTimer :public TimerBase
	{
	public:
		void Restart();
		void Stop();
		static void SleepSec(int sec);
		static void SleepSec(double sec);
		static void SleepMilliSec(int ms);
		static void SleepMilliSec(double ms);
	};
#endif

#pragma endregion

	/************************************************************************/
	/* data base															*/
	/************************************************************************/
#pragma region DATA_BASE

	/* NEED TODO: 需要存储的数据结构 */
	typedef struct SaveDataType
	{
		int a;
		float b;
		char c;
		double d;
	} T_SaveData;

	class Database
	{
	public:
		Database(std::string db_file);

		void LoadDbFile();
		void ResaveDbFile();
		void SaveRcd(T_SaveData rcd);
		void AppendRcd(T_SaveData rcd);

		std::map<std::string, T_SaveData> SaveDataMap;
		std::string GenKeyStr(T_SaveData rcd);
		T_SaveData Find(std::string key);

	private:
		std::string dbDirPath;
		std::string dbFileName;

		char checkSum(std::string key, T_SaveData rcd);
	};

#pragma endregion

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
#pragma region TASK

#define		TIMER_THREAD				(1)			// 是否使用单独的thread做定时器
#define		TASK_POOL_TICKS_PER_SEC		(100)		// 声明1秒调度节拍数

#define		TASK_STATE_DEAD				(-1)
#define		TASK_STATE_IDLE				(0)
#define		TASK_STATE_RUN				(1)
	typedef enum class TaskPoolStateEnum
	{
		HALT = 1,
		RUN = 2,
		IDLE = 3,
		BUSY = 4
	} E_TaskPoolState;
	typedef enum class SignalStateEnum
	{
		IDLE = 1,
		POSTED = 2,
		PENDING = 3,
		TIMEOUT = 4
	} E_SignalState;
	typedef enum class TaskPrioriEnum
	{
		LOW = 1,
		NORMAL = 2,
		HIGH = 3
	}E_TaskPriori;

	class TaskPool;
	class Task
	{
	public:
		Task(std::function <void(void)> func, E_TaskPriori priori, int64_t period_ms)
		{
			this->priori = priori;
			taskFunc = func;
			if (period_ms >= 0)
			{
				period = period_ms * TASK_POOL_TICKS_PER_SEC / 1000;
				delayCnt = period;
			}
			else
			{
				period = -1;
				delayCnt = -1;
			}
			state = TASK_STATE_IDLE;
		}

	protected:
		E_TaskPriori	priori;
		uint32_t		state;
		int64_t			period;
		int64_t			delayCnt;
		std::function <void(void)> taskFunc;

		friend class TaskPool;
	};
	class Signal
	{
	public:
		Signal(Task * pend_tsk)
		{
			state = E_SignalState::IDLE;
			pendTask = pend_tsk;
			timeOutClk = -1;
			sigCount = 0;
		}

		void Post()
		{
			sigCount++;
			state = E_SignalState::POSTED;
		}
		bool Pend(uint64_t time_out_ms)
		{
			if (sigCount > 0)
			{
				sigCount--;
				state = E_SignalState::IDLE;
				return true;
			}

			if (state == E_SignalState::IDLE)
			{
				timeOutClk = time_out_ms * TASK_POOL_TICKS_PER_SEC / 1000;;
				state = E_SignalState::PENDING;
			}
			return false;
		}

	protected:
		int64_t timeOutClk;
		Task * pendTask;
		int64_t sigCount;
		E_SignalState state;

		friend class TaskPool;
	};

	class TaskPool
	{
	public:
		TaskPool();
		~TaskPool();

		// period_ms = 0 : wake up every clk
		// period_ms = -1: not period, only waked up by signal
		Task * AddTask(std::function <void(void)> func, E_TaskPriori priori, int64_t period_ms = -1)
		{
			Task * task = new Task(func, priori, period_ms);
			switch (priori)
			{
			case E_TaskPriori::LOW:lowTasks.push_back(task); break;
			case E_TaskPriori::NORMAL:normalTasks.push_back(task); break;
			case E_TaskPriori::HIGH:highTasks.push_back(task); break;
			default:break;
			}

			return task;
		}
		Signal * AddSignal(Task * pend_tsk)
		{
			Signal * sig = new Signal(pend_tsk);
			signals.push_back(sig);
			return sig;
		}

		void Start()
		{
			init();
			while (1)
			{
				tasksSchedule();
			}
		}
		void TickIsr()
		{
			runTicCnt++;

			slicesTicCnt++;
			if (state == E_TaskPoolState::BUSY)
				busyTicCnt++;

			for (Task * task : highTasks)
			{
				if (task->state == TASK_STATE_DEAD)
					continue;

				if (task->delayCnt > 0)
					task->delayCnt--;

				if (task->delayCnt == 0)
				{
					task->state++;
					task->delayCnt = task->period;
				}
			}
			for (Task * task : normalTasks)
			{
				if (task->state == TASK_STATE_DEAD)
					continue;

				if (task->delayCnt > 0)
					task->delayCnt--;

				if (task->delayCnt == 0)
				{
					task->state++;
					task->delayCnt = task->period;
				}
			}
			for (Task * task : lowTasks)
			{
				if (task->state == TASK_STATE_DEAD)
					continue;

				if (task->delayCnt > 0)
					task->delayCnt--;

				if (task->delayCnt == 0)
				{
					task->state++;
					task->delayCnt = task->period;
				}
			}

			for (Signal * sig : signals)
			{
				if (sig->timeOutClk > 0)
					sig->timeOutClk--;

				// time out, do the pending task
				if (sig->state == E_SignalState::PENDING)
				{
					if (sig->timeOutClk == 0)
					{
						sig->pendTask->state++;
						sig->state = E_SignalState::TIMEOUT;
					}
				}

				// wake up no-period task
				if (sig->state == E_SignalState::POSTED)
				{
					if (sig->pendTask->delayCnt < 0)
					{
						sig->pendTask->state++;
						sig->state = E_SignalState::IDLE;
					}
				}
			}
		}

		size_t TaskNum() { return highTasks.size() + normalTasks.size() + lowTasks.size(); }
		double Usage() { return usage; }
		uint64_t RunTicks() { return runTicCnt; }

	protected:
		void init();
		void release();
		void tasksSchedule()
		{
			for (Task * task : highTasks)
			{
				if (task->state > 0)
				{
					state = E_TaskPoolState::BUSY;
					task->taskFunc();
					task->state--;
					state = E_TaskPoolState::IDLE;
					return;
				}
			}
			for (Task * task : normalTasks)
			{
				if (task->state > 0)
				{
					state = E_TaskPoolState::BUSY;
					task->taskFunc();
					task->state--;
					state = E_TaskPoolState::IDLE;
					return;
				}
			}
			for (Task * task : lowTasks)
			{
				if (task->state > 0)
				{
					state = E_TaskPoolState::BUSY;
					task->taskFunc();
					task->state--;
					state = E_TaskPoolState::IDLE;
					return;
				}
			}
			idleTask();
		}
		void idleTask()
		{
			if (slicesTicCnt >= 100)
			{
				usage = 1.0 * busyTicCnt / slicesTicCnt;
				busyTicCnt = 0;
				slicesTicCnt = 0;
			}
			//LOG("run tic = %lld, slices tic = %d, busy tic = %d, usage = %.4f.", runTicCnt, slicesTicCnt, busyTicCnt, usage);
		}

		std::vector<Task*> lowTasks;
		std::vector<Task*> normalTasks;
		std::vector<Task*> highTasks;
		std::vector<Signal*> signals;

		E_TaskPoolState	state;
		uint64_t		runTicCnt; // 运行总时长

		// 用于任务池使用率计算	
		double		usage;
		uint32_t	slicesTicCnt;
		uint32_t	busyTicCnt;
	};
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	extern void RunTaskPoolTest();

#pragma endregion

}
