#include "../inc/ff_utils.h"

namespace feifei
{
	CmdArgs * CmdArgs::pCmdArgs = nullptr;

	CmdArgs::CmdArgs(int argc, char *argv[])
	{
		argsNum = 0;
		argsMap = new std::map<E_ArgId, T_CmdArg*>();
		pCmdArgs = this;
		initCmdArgs();

		paserCmdArgs(argc, argv);
	}
	CmdArgs::CmdArgs()
	{
		argsNum = 0;
		argsMap = new std::map<E_ArgId, T_CmdArg*>();
		pCmdArgs = this;
		initCmdArgs();

		paserCmdArgs(0, nullptr);
	}
	CmdArgs * CmdArgs::GetCmdArgs()
	{
		if (pCmdArgs == nullptr)
			pCmdArgs = new CmdArgs();

		return pCmdArgs;
	}

	void * CmdArgs::GetOneArg(E_ArgId id)
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;
		it = argsMap->find(id);

		if (it == argsMap->end())
		{
			return nullptr;
		}

		return getOneArgValue(it->second);
	}

	void CmdArgs::addOneArg(E_ArgId id, E_DataType dType, std::string defaultVal, char sName, std::string lName, std::string tHelp)
	{
		T_CmdArg *arg = new T_CmdArg;

		arg->id = id;
		arg->type = dType;
		arg->value = defaultVal;
		arg->shortName = sName;
		arg->longName = lName;
		arg->helpText = tHelp;
		setOneArgValue(arg, defaultVal);

		argsMap->insert(std::pair<E_ArgId, T_CmdArg*>(id, arg));
		argsNum++;
	}

	/*
	 * 解析所有命令行参数
	 */
	void CmdArgs::paserCmdArgs(int argc, char *argv[])
	{
		if (argc == 0)
		{
			executePath = std::string(".") + DIR_SPT;
			return;
		}

		// use first arg to get current path
		size_t p;
		p = std::string(argv[0]).rfind(DIR_SPT);

		if (p == std::string::npos)
		{
			executePath = std::string(".") + DIR_SPT;
		}
		else
		{
			executePath = std::string(argv[0]).substr(0, p);
		}

		// other args
		for (int i = 1; i < argc; i++)
		{
			if ((std::string(argv[i]) == "--help") || (std::string(argv[i]) == "-h"))
			{
				helpText();
				exit(0);
			}
			else if ((argv[i][0] == '-') && (argv[i][1] == '-'))
			{
				if (argv[i + 1][0] != '-')
					setOneArg(std::string(&argv[i][2]), std::string(argv[i + 1]));
			}
			else if (argv[i][0] == '-')
			{
				if (argv[i + 1][0] != '-')
					setOneArg(argv[i][1], std::string(argv[i + 1]));
			}
		}
	}
	void CmdArgs::helpText()
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;

		for (it = argsMap->begin(); it != argsMap->end(); it++)
		{
			if (it->second->shortName != '\0')
				printf("-%c", it->second->shortName);
			else
				printf("  ");

			if (it->second->longName != "")
				printf(", --%s", it->second->longName.c_str());

			switch (it->second->type)
			{
			case E_DataType::Int:printf("(%d)", it->second->iValue); break;
			case E_DataType::Float:printf("(%.2f)", it->second->fValue); break;
			case E_DataType::String:printf("(%s)", it->second->sValue.c_str()); break;
			default: break;
			}

			if (it->second->helpText != "")
				printf(": %s", it->second->helpText.c_str());

			printf(".\n");
		}
	}

	/*
	* 根据命令行找到一个对应参数
	*/
	E_ReturnState CmdArgs::setOneArg(char sName, std::string value)
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;

		for (it = argsMap->begin(); it != argsMap->end(); it++)
		{
			if (it->second->shortName == sName)
				break;
		}

		if (it == argsMap->end())
		{
			FATAL("no such param -%c.", sName);
		}

		return setOneArgValue(it->second, value);
	}
	E_ReturnState CmdArgs::setOneArg(std::string lName, std::string value)
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;

		for (it = argsMap->begin(); it != argsMap->end(); it++)
		{
			if (it->second->longName == lName)
				break;
		}

		if (it == argsMap->end())
		{
			FATAL("no such param --%s.", lName.c_str());
		}

		return setOneArgValue(it->second, value);
	}

	/*
	* 根据命令行,设置/获取一个参数的值
	* 参数类型在初始化阶段设置
	*/
	E_ReturnState CmdArgs::setOneArgValue(T_CmdArg * arg, std::string value)
	{
		switch (arg->type)
		{
		case E_DataType::Int:
			arg->iValue = atoi(value.c_str());
			break;
		case E_DataType::Float:
			arg->fValue = atof(value.c_str());
			break;
		case E_DataType::String:
			arg->sValue = value;
			break;
		default:
			break;
		}
		return E_ReturnState::SUCCESS;
	}
	void * CmdArgs::getOneArgValue(T_CmdArg * arg)
	{
		switch (arg->type)
		{
		case E_DataType::Int:		return &(arg->iValue);
		case E_DataType::Float:		return &(arg->fValue);
		case E_DataType::String:	return &(arg->sValue);
		default:					return nullptr;
		}
	}
	 

	/************************************************************************/
	/* file and log															*/
	/************************************************************************/
	static std::string work_path = "";
	static std::string data_path = "";
	static std::string log_path = "";

	std::string get_curr_path()
	{
#ifdef _WIN32
		char * buf = _getcwd(nullptr, 0);
#else
		char * buf = getcwd(nullptr, 0);
#endif
		std::string path(buf);
		free(buf);
		return path;
	}
	std::string get_work_path()
	{
		init_work_path();
		return work_path;
	}
	std::string get_data_path()
	{
		init_work_path();
		return data_path;
	}
	void init_work_path()
	{
		if (work_path == "")
		{
			set_work_path(get_curr_path());
		}
	}
	void set_work_path(std::string path)
	{
		work_path = path + DIR_SPT;
		data_path = work_path + "data" + DIR_SPT;
		log_path = work_path + std::string("log") + DIR_SPT;
		ensure_dir(work_path.c_str());
		ensure_dir(data_path.c_str());
		ensure_dir(log_path.c_str());
	}
	void ensure_dir(const char * dir)
	{
#ifdef _WIN32
		if (_access(dir, 2) == -1)
		{
			_mkdir(dir);
		}
#else
		if (access(dir, F_OK) == -1)
		{
			::mkdir(dir, 0777);
		}
#endif
	}

	std::string get_file_path(std::string fileName)
	{
		size_t p;
		p = fileName.rfind(DIR_SPT);
		return fileName.substr(0, p);
	}
	std::string get_file_name(std::string fileName)
	{
		size_t p1, p2;
		p1 = fileName.rfind(DIR_SPT);
		p2 = fileName.rfind(".");
		return fileName.substr(p1 + 1, p2 - p1 - 1);
	}

	void exec_cmd(std::string cmd)
	{
#ifdef _WIN32
		system(cmd.c_str());
#else
		FILE * pfp = popen(cmd.c_str(), "r");
		auto status = pclose(pfp);
		WEXITSTATUS(status);
#endif
	}

	E_ReturnState dump2_txt_file(std::string file_name, std::string str)
	{
		std::ofstream fout(file_name.c_str(), std::ios::out | std::ios::ate);
		if (!fout.is_open())
		{
			ERR("can't open save file: " + file_name);
		}
		fout.write(str.c_str(), str.length());
		fout.close();
		return E_ReturnState::SUCCESS;
	}

	/************************************************************************/
	/* file and log															*/
	/************************************************************************/
	std::string fmtTime(double sec)
	{
		char * p;
		if (sec >= 1.0)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(sec)", sec);
			return std::string(p);
		}
		else if (sec >= 1e-3)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(ms)", sec * 1000.0);
			return std::string(p);
		}
		else
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(us)", sec * 1000.0 * 1000.0);
			return std::string(p);
		}
	}
	std::string fmtFreq(uint64_t freqHz)
	{
		char * p;
		if (freqHz >= 1000 * 1000 * 1000)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.1f(GHz)", freqHz / 1000.0 / 1000.0 / 1000.0);
			return std::string(p);
		}
		else if (freqHz >= 1000 * 1000)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.1f(MHz)", freqHz / 1000.0 / 1000.0);
			return std::string(p);
		}
		else if (freqHz >= 1000)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.1f(KHz)", freqHz / 1000.0);
			return std::string(p);
		}
		else
		{
			p = (char*)alloca(128);
			sprintf(p, "%lld(Hz)", freqHz);
			return std::string(p);
		}
	}
	std::string fmtSize(size_t szByte)
	{
		char * p;
		if (szByte >= (size_t)1024 * 1024 * 1024 * 1024)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(TB)", szByte / 1024.0 / 1024.0 / 1024.0 / 1024.0);
			return std::string(p);
		}
		else if (szByte >= 1024 * 1024 * 1024)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(GB)", szByte / 1024.0 / 1024.0 / 1024.0);
			return std::string(p);
		}
		else if (szByte >= 1024 * 1024)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(MB)", szByte / 1024.0 / 1024.0);
			return std::string(p);
		}
		else if (szByte >= 1024)
		{
			p = (char*)alloca(128);
			sprintf(p, "%.3f(KB)", szByte / 1024.0);
			return std::string(p);
		}
		else
		{
			p = (char*)alloca(128);
			sprintf(p, "%zd(Byte)", szByte);
			return std::string(p);
		}
	}

	static const int PrintBufferSize = 1024;
	static char PrintBuffer[PrintBufferSize];

	static time_t t;
#ifdef _WIN32
	static struct tm stm;
#endif
	static std::string getCurrentTime()
	{
		memset(PrintBuffer, 0, PrintBufferSize);
		t = time(0);
#ifdef _WIN32
		localtime_s(&stm, &t);
		strftime(PrintBuffer, PrintBufferSize, "[%H:%M:%S]", &stm);
#else
		strftime(PrintBuffer, PrintBufferSize, "[%H:%M:%S]", localtime(&t));
#endif
		return std::string(PrintBuffer);
	}

	static const int CommentLength = 73;
	void PrintSeperator(const char c, std::ostream *sm)
	{
		*sm << std::string(CommentLength, c) << std::endl;
	}

	void PrintInfo(const char * format, ...)
	{
		memset(PrintBuffer, 0, PrintBufferSize);
		va_list args;
		va_start(args, format);
#ifdef _WIN32
		vsprintf_s(PrintBuffer, PrintBufferSize, format, args);
#else
		vsprintf(PrintBuffer, format, args);
#endif
		printf("%s", PrintBuffer);
		va_end(args);
		printf("\n");
	}
	void PrintInfo(std::string msg, ...)
	{
		printf(msg.c_str());
		printf("\n");
	}

	void PrintLog(const char * format, ...)
	{
		std::cout << "[  LOG  ]" << getCurrentTime() << std::flush;

		memset(PrintBuffer, 0, PrintBufferSize);
		va_list args;
		va_start(args, format);
#ifdef _WIN32
		vsprintf_s(PrintBuffer, PrintBufferSize, format, args);
#else
		vsprintf(PrintBuffer, format, args);
#endif
		va_end(args);
		std::cout << std::string(PrintBuffer) << std::endl;
	}
	void PrintLog(std::string msg, ...)
	{
		std::cout << "[  LOG  ]" << getCurrentTime() << msg << std::endl;
	}

	void PrintWarning(const char *file, int line, const char * format, ...)
	{
		std::cout << "[WARNING]" << getCurrentTime() << std::flush;

		int pos;
		char * p = (char *)file;
		memset(PrintBuffer, 0, PrintBufferSize);
		va_list args;
		va_start(args, format);
#ifdef _WIN32
		pos = vsprintf_s(PrintBuffer, PrintBufferSize, format, args);
		p = strrchr(p, DIR_SPT) + 1;
		sprintf_s(PrintBuffer + pos, PrintBufferSize, " @%s:%d", p, line);
#else
		pos = vsprintf(PrintBuffer, format, args);
		p = strrchr(p, DIR_SPT) + 1;
		sprintf(PrintBuffer + pos, " @%s:%d", p, line);
#endif
		va_end(args);
		std::cout << std::string(PrintBuffer) << std::endl;
	}
	void PrintWarning(const char *file, int line, std::string msg, ...)
	{
		std::cout << "[WARNING]" << getCurrentTime() << msg << std::flush;

		std::string sf(file);
		std::cout << " @" << sf.erase(0, sf.find_last_of(DIR_SPT) + 1)
			<< ":" << line << std::endl;
	}

	E_ReturnState PrintError(const char *file, int line, const char * format, ...)
	{
		std::cerr << "[ERROR]" << getCurrentTime() << std::flush;

		int pos;
		char * p = (char *)file;
		memset(PrintBuffer, 0, PrintBufferSize);
		va_list args;
		va_start(args, format);
#ifdef _WIN32
		pos = vsprintf_s(PrintBuffer, PrintBufferSize, format, args);
		p = strrchr(p, DIR_SPT) + 1;
		sprintf_s(PrintBuffer + pos, PrintBufferSize, " @%s:%d", p, line);
#else
		pos = vsprintf(PrintBuffer, format, args);
		p = strrchr(p, DIR_SPT) + 1;
		sprintf(PrintBuffer + pos, " @%s:%d", p, line);
#endif
		va_end(args);
		std::cerr << std::string(PrintBuffer) << std::endl;

		return E_ReturnState::RTN_ERR;
	}
	E_ReturnState PrintError(const char *file, int line, std::string msg, ...)
	{
		std::cerr << "[ERROR]" << getCurrentTime() << msg << std::flush;

		std::string sf(file);

		std::cerr << " @" << sf.erase(0, sf.find_last_of(DIR_SPT) + 1)
			<< ":" << line << std::endl;

		return E_ReturnState::RTN_ERR;
	}

	void PrintFatal(const char *file, int line, const char * format, ...)
	{
		std::cerr << "[RTN_FATAL]" << getCurrentTime() << std::flush;

		int pos;
		char * p = (char *)file;
		memset(PrintBuffer, 0, PrintBufferSize);
		va_list args;
		va_start(args, format);
#ifdef _WIN32
		pos = vsprintf_s(PrintBuffer, PrintBufferSize, format, args);
		p = strrchr(p, DIR_SPT) + 1;
		sprintf_s(PrintBuffer + pos, PrintBufferSize, " @%s:%d", p, line);
#else
		pos = vsprintf(PrintBuffer, format, args);
		p = strrchr(p, DIR_SPT) + 1;
		sprintf(PrintBuffer + pos, " @%s:%d", p, line);
#endif
		va_end(args);
		std::cerr << std::string(PrintBuffer) << std::endl;

		exit(EXIT_FAILURE);
	}
	void PrintFatal(const char *file, int line, std::string msg, ...)
	{
		std::cerr << "[RTN_FATAL]" << getCurrentTime() << msg << std::flush;

		std::string sf(file);

		std::cerr << " @" << sf.erase(0, sf.find_last_of(DIR_SPT) + 1)
			<< ":" << line << std::endl;

		exit(EXIT_FAILURE);
	}

#define chkFStream() do{\
		if ((fstream == nullptr)||(!fstream->is_open())){\
			WARN("can't open log file");\
			fstream = nullptr;\
			return;\
		}}while(0)
}
