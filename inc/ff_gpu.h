#pragma once

//#define	 __GPU_RT_HIP
//#define	 __GPU_RT_OCL
//#define	 __GPU_RT_CU_DRV

#include "ff_utils.h"

#ifdef __GPU_RT_HIP
#define EN_AUTO_GEN 1
#define EN_SP3_AUTO_GEN 0
#endif

namespace feifei
{
#define MAX_ARGS_SIZE		1024

	extern void init_kernel_path();
	extern std::string get_kernel_path();
	extern void set_kernel_path(std::string path);

	typedef enum class GpuRtEnum
	{
		OCL = 1,
		CU_DRV,
		HSA,
		HIP,
		CUDA
	} E_GpuRt;
	typedef enum class ProgramTypeEnum
	{
		CPP_FILE,
		CPP_STRING,
		GAS_FILE,
		GAS_STRING,
		PTX_FILE,
		PTX_STRING,
		BIN_FILE,
		BIN_ARRAY
	}E_ProgramType;
	typedef enum class IsaArchEnum
	{
		Cuda60 = 60,
		Cuda61 = 61,
		Cuda62 = 62,
		Cuda70 = 70,
		Cuda72 = 72,
		Cuda75 = 75,

		Gfx803 = 803,	// Fiji/MI8
		Gfx900 = 900,	// Vega10/MI25
		Gfx906 = 906,	// Vega20/MI50/MI60
		Gfx908 = 908	// MI100
	}E_IsaArch;
	typedef enum class MemTypeEnum
	{
		Page = 1,
		Pin = 2,
		WrCmb = 3,
		Map = 4,
		Pitch = 5,
		Dev = 6
	}E_MemType;

	typedef struct dim
	{
		size_t x, y, z;
		dim(size_t vx = 1, size_t vy = 1, size_t vz = 1) : x(vx), y(vy), z(vz) {}
		size_t * arr() { return new size_t[3]{ x,y,z }; }

		dim operator/(dim b)
		{
			size_t x2 = this->x / b.x;
			size_t y2 = this->y / b.y;
			size_t z2 = this->z / b.z;
			dim c(x2, y2, z2);
			return c;
		}
		dim operator*(dim b)
		{
			size_t x2 = this->x * b.x;
			size_t y2 = this->y * b.y;
			size_t z2 = this->z * b.z;
			dim c(x2, y2, z2);
			return c;
		}
	}dim;

	typedef struct PlatformInfoType
	{
		std::string name;
		std::string version;
		std::string vendor;
	}T_PlatformInfo;
	typedef struct DeviceInfoType
	{
		std::string Name;
		E_IsaArch Arch;

		uint64_t CoreClkFreqHz;
		int CuNum;
		int SimdNumPerCu;
		int TotalSimdNum;

		int WaveSize;

		uint64_t MemClkFreqHz;
		size_t LdsSize;
		size_t GlobalMemSize;
		int GlobalMemWidth;
		int L2CacheSize;
	} T_DeviceInfo;

	class GpuKernelBase
	{
	protected:
		std::string compiler;
		std::string buildOption = "";

		E_ProgramType programType;	// kernel源程序类型
		std::string programFile;	// kernel源程序文件
		char * programSrc;			// kernel源程序字符串（或）
		size_t programSize;

		std::string kernelName;		// kernel函数名字
		std::string kernelFile;		// 编译出的二进制文件
		size_t kernelSize;

		size_t argsSize = 0;
		uint8_t argsBuff[MAX_ARGS_SIZE];
		E_ReturnState setArgs() { return E_ReturnState::SUCCESS; }
		template <typename T, typename... Ts> E_ReturnState setArgs(T head, Ts... rest)
		{
			//unsigned int argsCnt = sizeof...(rest);
			T param = head;
			memcpy(argsBuff + argsSize, &param, sizeof(head));
			argsSize += sizeof(head);
			setArgs(rest...);

			return E_ReturnState::SUCCESS;
		}

		virtual E_ReturnState creatKernelFromCppFile() = 0;
		virtual E_ReturnState creatKernelFromCppString() = 0;
		virtual E_ReturnState creatKernelFromAsmFile() = 0;
		virtual E_ReturnState creatKernelFromAsmString() = 0;
		virtual E_ReturnState creatKernelFromBinFile() = 0;
		virtual E_ReturnState creatKernelFromBinArray() = 0;
		virtual E_ReturnState dumpSource() = 0;
		virtual E_ReturnState dumpAssembly() = 0;
		virtual E_ReturnState dumpBinary() = 0;

	public:
		GpuKernelBase() {}
		virtual GpuKernelBase * NewCopy() = 0;

		void ResetArg() { argsSize = 0; }
		template <typename T> E_ReturnState SetArg(T param)
		{
			T _param = param;
			if (argsSize + sizeof(T) >= MAX_ARGS_SIZE)
				return E_ReturnState::RTN_ERR;
			memcpy(argsBuff + argsSize, &_param, sizeof(T));
			argsSize += sizeof(T);
			return E_ReturnState::SUCCESS;
		}
		template <typename... Ts> E_ReturnState SetArgs(Ts... param)
		{
			argsSize = 0;
			setArgs(param...);
			return E_ReturnState::SUCCESS;
		}
	};

	class GpuStreamBase
	{
	protected:
		bool enProf;
		double kernelExeTime;

	public:
		virtual E_ReturnState Launch(GpuKernelBase *k, dim global_sz, dim group_sz) = 0;
		virtual void Flush() = 0;

		virtual E_ReturnState MemCopyH2DAsync(void * d_mem, void * h_mem, size_t byteNum) = 0;
		virtual E_ReturnState MemCopyD2HAsync(void * h_mem, void * d_mem, size_t byteNum) = 0;
		virtual E_ReturnState MemCopyD2DAsync(void * d_dst, void * d_src, size_t byteNum) = 0;

		double KernelExeTime() { return kernelExeTime; }
	};

	class GpuDeviceBase
	{
	protected:
		friend class GpuRuntimeBase;
		friend class GpuKernelBase;
		friend class GpuStreamBase;

	protected:
		virtual E_ReturnState initDevice() = 0;
		virtual void deInitDevice() = 0;

		uint64_t platformId;
		uint64_t deviceId;

		std::vector<GpuStreamBase*> streams;
		std::vector<GpuKernelBase*> kernels;

		T_DeviceInfo * deviceInfo;
		virtual void getDeviceInfo() = 0;

	public:
		GpuDeviceBase(uint64_t id)
		{
			deviceId = id;
		}

		T_DeviceInfo DeviceInfo() { return *deviceInfo; }
		void PrintDeviceInfo()
		{
			PrintSeperator('-');

			double perf = 2.0 * deviceInfo->TotalSimdNum * deviceInfo->CoreClkFreqHz;	// 2 opts(mult & add) in one cycle
			INFO("- Device Name: " + deviceInfo->Name);
			INFO("- Compute Unit Number = %d", deviceInfo->CuNum);
			INFO("- Shading Core per CU = %d", deviceInfo->SimdNumPerCu);
			INFO("- Core Clock: " + fmtFreq(deviceInfo->CoreClkFreqHz));
			INFO("- Performance(fp32) = %.3f(TFlops)", perf * 1e-12);
			INFO("- Memory Clock: " + fmtFreq(deviceInfo->MemClkFreqHz));
			INFO("- Shared Memory per Block: " + fmtSize(deviceInfo->LdsSize));
			INFO("- Total Global Memory: " + fmtSize(deviceInfo->GlobalMemSize));
			PrintSeperator('-');
		}
		virtual void PrintDeviceDetails() = 0;

		void AddKernel(GpuKernelBase * k) { kernels.push_back(k); }
		void AddStream(GpuStreamBase * s) { streams.push_back(s); }
	};

	class GpuRuntimeBase
	{
	protected:
		GpuRuntimeBase()
		{
			kernelTempDir = get_work_path() + DIR_SPT + "kernel";
			ensure_dir(kernelTempDir.c_str());
		}
		virtual E_ReturnState initPlatform() = 0;
		virtual void getPlatformInfo() = 0;

		uint64_t platformId;
		T_PlatformInfo platformInfo;

		std::vector<GpuDeviceBase*> devices;
		GpuDeviceBase * selDevice;
		GpuStreamBase * defaultStream = nullptr;

		std::string runtimeName;
		std::string kernelTempDir;

	public:
		void PrintRuntimeInfo(bool isFullInfo = false)
		{
			PrintSeperator('=');
			INFO("= " + runtimeName + " Info:");
			PrintSeperator('=');

			PrintPlatformInfo();

			for (int i = 0; i < DevicesCnt(); i++)
			{
				if (isFullInfo)
				{
					devices[i]->PrintDeviceDetails();
				}
				else
				{
					devices[i]->PrintDeviceInfo();
				}
			}
		}
		void PrintPlatformInfo()
		{
			INFO("- Platform Name: " + platformInfo.name);
			INFO("- Version: " + platformInfo.version);
			INFO("- Vendor Name: " + platformInfo.vendor);
		}
		T_PlatformInfo * PlatformInfo() { return &platformInfo; }

		size_t DevicesCnt() { return devices.size(); }
		GpuDeviceBase * Device() { return selDevice; }
		E_ReturnState SellectDevice(unsigned int devNum)
		{
			if (devNum >= devices.size())
				return E_ReturnState::RTN_ERR;
			selDevice = devices[devNum];
			selDevice->initDevice();

			// create default stream on sellected device
			if (defaultStream == nullptr)
			{
				defaultStream = CreateStream();
			}
			return E_ReturnState::SUCCESS;
		}
		GpuStreamBase * GetDefaultStream()
		{
			if (defaultStream == nullptr)
			{
				defaultStream = CreateStream();
			}
			return defaultStream;
		}

		virtual GpuStreamBase * CreateStream(bool enProf = true, int devNum = -1) = 0;
		virtual GpuKernelBase * CreateKernel(char * content, std::string kernelName, E_ProgramType type, int devNum = -1) = 0;

		virtual void* DevMalloc(size_t byteNum) = 0;
		virtual void* HostMalloc(size_t byteNum, E_MemType memType = E_MemType::Page) = 0;
		virtual void DevFree(void* d_mem) = 0;
		virtual void HostFree(void* h_mem, E_MemType memType = E_MemType::Page) = 0;
		virtual E_ReturnState MemCopyH2D(void * d_mem, void * h_mem, size_t byteNum) = 0;
		virtual E_ReturnState MemCopyD2H(void * h_mem, void * d_mem, size_t byteNum) = 0;
		virtual E_ReturnState MemCopyD2D(void * d_dst, void * d_src, size_t byteNum) = 0;

		void SetKernelTempDir(std::string dir)
		{
			kernelTempDir = dir;
			ensure_dir(kernelTempDir.c_str());
		}
		std::string KernelTempDir() { return kernelTempDir; }

	};

	typedef struct DispatchType
	{
	public:
		DispatchType();
		DispatchType(GpuStreamBase * stream, dim global_size, dim group_size);
		DispatchType(dim global_size, dim group_size);
		GpuStreamBase * stream;
		GpuKernelBase * kernel;
		dim global_size;
		dim group_size;
	}T_Dispatch;
}

#ifdef __GPU_RT_OCL
#include "ff_ocl_runtime.h"
#define GPU_RUNTIME_TEMP E_GpuRt::OCL
#endif
#ifdef __GPU_RT_CU_DRV
#include "ff_cu_drv_runtime.h"
#define GPU_RUNTIME_TEMP E_GpuRt::CU_DRV
#endif
#ifdef __GPU_RT_HSA
#include "ff_hsa_runtime.h"
#define GPU_RUNTIME_TEMP E_GpuRt::HSA
#endif
#ifdef __GPU_RT_HIP
#include "ff_hip_runtime.h"
#define GPU_RUNTIME_TEMP E_GpuRt::HIP
#endif

namespace feifei
{
	enum class OpTargetEnum
	{
		NV = 1,
		AMD = 2,
		CPU = 3
	};
	template <OpTargetEnum T> struct OpTarget {};
	typedef OpTarget<OpTargetEnum::NV> NV;
	typedef OpTarget<OpTargetEnum::AMD> AMD;
	typedef OpTarget<OpTargetEnum::CPU> CPU;

	class GpuRuntime
	{
	public:
		static E_GpuRt GpuRtType();
		static GpuRuntimeBase * GetInstance();
		static GpuRuntimeBase * GetInstance(E_GpuRt rtType);
	};

#pragma region DATA_MEM
	template <typename DataType> class DataMem
	{
	public:
		~DataMem()
		{
			if (hst_addr != 0)
			{
				if (mem_type == E_MemType::Dev)
				{
					free(hst_addr);
				}
				else if (mem_type == E_MemType::Page)
				{
					free(hst_addr);
				}
				else
				{
					GpuRuntimeBase * rt = GpuRuntime::GetInstance();
					rt->HostFree(dev_addr);
				}
			}
			if (dev_addr != 0)
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				rt->DevFree(dev_addr);
			}
		}
		DataMem(std::string name = "", E_MemType mem_type = E_MemType::Page)
		{
			hst_addr = nullptr;	dev_addr = nullptr;
			data_count = 0;		mem_size = 0;
			this->name = name;
			this->mem_type = mem_type;
		}
		template <typename... T> DataMem(std::string name, E_MemType mem_type, T... size)
		{
			hst_addr = nullptr;	dev_addr = nullptr;
			data_count = 0;		mem_size = 0;

			this->name = name;
			this->mem_type = mem_type;

			dataSize(size...);

			if (mem_type == E_MemType::Dev)
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				dev_addr = rt->DevMalloc(mem_size);
			}
			else if (mem_type == E_MemType::Map)
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				hst_addr = rt->HostMalloc(mem_size);
			}
			else
			{
				hst_addr = malloc(mem_size);
			}
		}
		template <typename... T> DataMem(std::string name, void * addr, E_MemType mem_type, T... size)
		{
			hst_addr = nullptr;	dev_addr = nullptr;
			data_count = 0;		mem_size = 0;
			this->name = name;

			this->mem_type = mem_type;
			if (mem_type == E_MemType::Dev)
			{
				dev_addr = addr;
			}
			else
			{
				hst_addr = addr;
			}

			dataSize(size...);
		}

	public:
		void Log(E_DataFormat fmt = E_DataFormat::Nomal, int fmtLen = 2, uint64_t startIdx = 0, uint64_t endIdx = 0, int numPerRow = 8)
		{
			if (mem_type == E_MemType::Dev)
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				if (hst_addr == NULL)
					hst_addr = malloc(mem_size);
				rt->MemCopyD2H(hst_addr, dev_addr, mem_size);
			}
			log_data_mem<DataType>(hst_addr, data_count, name, fmt, fmtLen, startIdx, endIdx, numPerRow, &data_size);
		}
		void LogHst(E_DataFormat fmt = E_DataFormat::Nomal, int fmtLen = 2, uint64_t startIdx = 0, uint64_t endIdx = 0, int numPerRow = 8)
		{
			log_data_mem<DataType>(hst_addr, data_count, name, fmt, fmtLen, startIdx, endIdx, numPerRow, &data_size);
		}
		void LogDev(E_DataFormat fmt = E_DataFormat::Nomal, int fmtLen = 2, uint64_t startIdx = 0, uint64_t endIdx = 0, int numPerRow = 8)
		{
			GpuRuntimeBase * rt = GpuRuntime::GetInstance();
			if (hst_addr == NULL)
				hst_addr = malloc(mem_size);
			rt->MemCopyD2H(hst_addr, dev_addr, mem_size);

			log_data_mem<DataType>(hst_addr, data_count, name, fmt, fmtLen, startIdx, endIdx, numPerRow, &data_size);
		}
		E_ReturnState Dump(E_DataFormat fmt = E_DataFormat::Nomal, int fmtLen = 0, uint64_t startIdx = 0, uint64_t endIdx = 0, int numPerRow = 8)
		{
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState DumpBin()	
		{
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState LoadBin(std::string file_name, uint64_t load_cnt = 0)
		{
			return E_ReturnState::SUCCESS;
		}

		template <typename... T> E_ReturnState ReSize(T... size)
		{
			size_t old_data_cnt = data_count;
			size_t old_mem_size = mem_size;

			data_size.clear();
			dataSize(size...);

			if (old_data_cnt == data_count)
				return E_ReturnState::SUCCESS;

			void * old_dev_addr = dev_addr;
			void * old_hst_addr = hst_addr;

			mem_size = data_count * sizeof(DataType);

			if ((old_dev_addr != nullptr) || (mem_type == E_MemType::Dev))
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				dev_addr = rt->DevMalloc(mem_size);
				rt->MemCopyD2D(dev_addr, old_dev_addr, old_mem_size);
			}
			if ((old_hst_addr != nullptr) || (mem_type != E_MemType::Dev))
			{
				hst_addr = malloc(mem_size);
				memcpy(hst_addr, old_hst_addr, old_mem_size);
			}
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState ReSize(std::vector<uint64_t> sz)
		{
			size_t old_data_cnt = data_count;
			size_t old_mem_size = mem_size;

			data_size.clear();
			data_dim = sz.size();
			data_count = 1;
			for (uint64_t s : sz)
			{
				data_size.push_back(s);
				data_count *= s;
			}
			mem_size = sizeof(DataType) * data_count;

			if (old_data_cnt == data_count)
				return E_ReturnState::SUCCESS;

			void * old_dev_addr = dev_addr;
			void * old_hst_addr = hst_addr;

			if ((old_dev_addr != nullptr) || (mem_type == E_MemType::Dev))
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				dev_addr = rt->DevMalloc(mem_size);
				rt->MemCopyD2D(dev_addr, old_dev_addr, old_mem_size);
			}
			if ((old_hst_addr != nullptr) || (mem_type != E_MemType::Dev))
			{
				hst_addr = malloc(mem_size);
				memcpy(hst_addr, old_hst_addr, old_mem_size);
			}
			return E_ReturnState::SUCCESS;
		}

		E_ReturnState Sync2Hst()
		{
			if (mem_type == E_MemType::Dev)
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				if (hst_addr == NULL)
					hst_addr = malloc(mem_size);
				rt->MemCopyD2H(hst_addr, dev_addr, mem_size);
			}
			return E_ReturnState::SUCCESS;
		}
		E_ReturnState Sync2Dev()
		{
			if (mem_type != E_MemType::Dev)
			{
				GpuRuntimeBase * rt = GpuRuntime::GetInstance();
				if (dev_addr == NULL)
					dev_addr = rt->DevMalloc(mem_size);
				rt->MemCopyH2D(dev_addr, hst_addr, mem_size);
			}
			return E_ReturnState::SUCCESS;
		}

		void * operator+(uint64_t idx)
		{
			return hst_addr + sizeof(DataType) * idx;
		}
		DataType operator[](uint64_t idx)
		{
			data_ptr = (DataType*)hst_addr;
			return *(data_ptr + idx);
		}

		std::string Name() { return name; }
		void * HstAddr() { return hst_addr; }
		void * DevAddr() { return dev_addr; }
		E_MemType MemType() { return mem_type; }
		void SetMemType(E_MemType t) { mem_type = t; }
		std::vector<uint64_t> DataSize() { return data_size; }
		uint64_t DataDim() { return data_size.size(); }
		uint64_t DataCount() { return data_count; }
		size_t MemSize() { return mem_size; }
		void SetHstAddr(void * addr) { hst_addr = addr; }
		void SetDevAddr(void * addr) { dev_addr = addr; }

	private:
		void dataSize()
		{
			data_dim = data_size.size();
			data_count = 1;
			for (uint64_t size : data_size)
			{
				data_count *= size;
			}
			mem_size = sizeof(DataType) * data_count;
		}
		template <typename... T> void dataSize(uint64_t head_size, T... rest_size)
		{
			unsigned int size_cnt = sizeof...(rest_size);

			data_size.push_back(head_size);
			dataSize(rest_size...);
		}

		std::string name;

		DataType * data_ptr;
		void * hst_addr;
		void * dev_addr;
		E_MemType mem_type;

		std::vector<uint64_t> data_size;
		uint64_t data_dim;
		uint64_t data_count;
		size_t mem_size;
	};
	extern void DataMemTest();
	extern void BandwidthTest();
	
	template <typename T> void CompareDataMem(DataMem<T> * rslt, DataMem<T> * ref, uint64_t verify_len = 0)
	{
		LOG("Verify Operation Result: %s VS %s", rslt->Name().c_str(), ref->Name().c_str());
		ref->Sync2Hst();
		rslt->Sync2Hst();
		if (memcmp(rslt->HstAddr(), ref->HstAddr(), ref->MemSize()) == 0)
		{
			LOG("Verify Memory Success.");
			return;
		}

		verify_len = (verify_len == 0) ? ref->DataCount() : verify_len;
		double total_diff = 0;
		for (uint64_t i = 0; i < verify_len; i++)
		{
			total_diff += ((*ref)[i] - (*rslt)[i]) * ((*ref)[i] - (*rslt)[i]);
		}
		double mean_diff = total_diff / ref->DataCount();

		if (!(mean_diff >= 0 && mean_diff < MIN_FP32_ERR))
		{
			WARN("Verify Failed! total err = %f", total_diff);
			WARN("Verify Failed! mean err = %f", mean_diff);
		}
		else
		{
			LOG("Verify Success.");
		}
	}
	//extern void CompareDataMem(DataMem<cplx_fp32> * rslt, DataMem<cplx_fp32> * ref, uint64_t verify_len = 0);
	extern DataMem<float> * InitRealData(std::string name, float init_type, uint32_t dim0, uint32_t dim1 = 1, uint32_t dim2 = 1);
	//extern DataMem<cplx_fp32> * InitCplxData(std::string name, uint32_t dim0, uint32_t dim1 = 1, uint32_t dim2 = 1);
#pragma endregion

#pragma region TEST_FW
	typedef struct ScoreTypde
	{
		double TheoryElapsedTime; //(s)
		double TheoryFlops;		//(Flops)

		double ElapsedTime;	//(s)
		double Flops;		//(Flops)
		double Calculation;

		double Performence;	//(%)
	}T_Score;

	/************************************************************************/
	/* solution 控制										                    */
	/************************************************************************/
	class SolutionCtrlBase
	{
	public:
		SolutionCtrlBase(std::string name = "");
		~SolutionCtrlBase();
		void RunSolution();

	protected:
		std::string solutionName;

		bool isCombKernels = true;
		GpuStreamBase * default_stream;
		std::vector<GpuKernelBase*> kernels;
		std::vector<T_Dispatch> dispatches;

		int repeatTimes;
		T_Score score;
		double elapsedTime = 0;

		virtual E_ReturnState generateKernel();
		template <typename... Ts> void setParam(GpuKernelBase * kernel, Ts... param)
		{
			kernel->ResetArg();
			setOpParam(kernel, param...);
		}
		virtual E_ReturnState verifyResult();

	private:
		E_ReturnState launchSingleKernel(unsigned int idx);
		E_ReturnState launchKernel();
		void run_comb_kernel();
		void run_mult_kernel();
		void setOpParam(GpuKernelBase * kernel) { }
		template <typename T, typename... Ts> void setOpParam(GpuKernelBase * kernel, T head_param, Ts... rest_param)
		{
			kernel->SetArg(head_param);
			setOpParam(kernel, rest_param...);
			//kernel->SetArgs(head_param, rest_param...);
		}
		template <typename Tdata, typename... Ts> void setOpParam(GpuKernelBase * kernel, DataMem<Tdata> * data, Ts... rest_param)
		{
			kernel->SetArg(data->DevAddr());
			setOpParam(kernel, rest_param...);
		}
	};

	/************************************************************************/
	/* solver 控制															*/
	/************************************************************************/
	class SolverCtrlBase
	{
	public:
		SolverCtrlBase();
		~SolverCtrlBase();
		void RunSolver();

	protected:
		std::vector<SolutionCtrlBase*> solutions;

		virtual void generateSolver();
	};

	/************************************************************************/
	/* problem 控制															*/
	/************************************************************************/
	class ProblemCtrlBase
	{
	public:
		ProblemCtrlBase(std::string name = "");
		~ProblemCtrlBase();
		void RunProblem();

	protected:
		std::string problemName;
		SolverCtrlBase * solver;

		std::vector<DataMem<void*>*> dataMems;
		template <typename T, typename... Tsz> DataMem<T> * newDataMem(std::string name, void * addr, E_MemType mem_type, Tsz... sz)
		{
			DataMem<T> * data_mem;

			if (addr == nullptr)
				data_mem = new DataMem<T>(name, mem_type, sz...);
			else
				data_mem = new DataMem<T>(name, addr, mem_type, sz...);

			DataMem<void*> * t_data = (DataMem<void*>*)data_mem;
			dataMems.push_back(t_data);
			return data_mem;
		}
		template <typename T> DataMem<T> * newRealData(std::string name, float init_type, uint32_t dim0, uint32_t dim1 = 1, uint32_t dim2 = 1)
		{
			DataMem<T> * data_mem = InitRealData(name, init_type, dim0, dim1, dim2);
			DataMem<void*> * t_data = (DataMem<void*>*)data_mem;
			dataMems.push_back(t_data);
			return data_mem;
		}
		//DataMem<cplx_fp32> * newCplxData(std::string name, uint32_t dim0, uint32_t dim1 = 1, uint32_t dim2 = 1);

		virtual void initDataMem();
		virtual void cpuCompute();
	};
	
	/************************************************************************/
	/* 添加测试																*/
	/************************************************************************/
#define TEST(T)  do{T *m##T = new T(); addTestExample(m##T);}while(0)
	extern void addTestExample(ProblemCtrlBase * p);
	extern void RunGpuExample();

#pragma endregion
}

#ifdef __GPU_RT_HIP
#if EN_AUTO_GEN
#include "IsaGenerater.h"
#endif
#if EN_SP3_AUTO_GEN
#include "KernelWriter.h"
#endif
#endif
