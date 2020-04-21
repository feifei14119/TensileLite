#include "../inc/ff_gpu.h"

namespace feifei
{
	std::string g_KernelPath;

	RuntimeHip * RuntimeHip::pInstance = nullptr;
	RuntimeHip * RuntimeHip::GetInstance()
	{
		if (pInstance == nullptr)
		{
			pInstance = new RuntimeHip();

			if (pInstance->initPlatform() != E_ReturnState::SUCCESS)
			{
				pInstance = nullptr;
			}
		}

		return pInstance;
	}

	E_ReturnState RuntimeHip::initPlatform()
	{
		hipError_t errNum;

		runtimeName = "HIP-HCC";
		hipInit(0);

		// device
		int deviceCnt;
		errNum = hipGetDeviceCount(&deviceCnt);
		if (errNum != hipSuccess || deviceCnt <= 0)
		{
			FATAL("no hip device support.");
		}

		devices.clear();
		for (unsigned int i = 0; i < deviceCnt; i++)
		{
			DeviceHip *dev = new DeviceHip(i);
			devices.push_back(dev);
		}

		hipDevice_t device;
		hipCtx_t context;
		hipDeviceGet(&device, 0);
		hipCtxCreate(&context, 0, device);

		return E_ReturnState::SUCCESS;
	}

	void RuntimeHip::getPlatformInfo()
	{
	}
	void DeviceHip::getDeviceInfo()
	{
		hipDevInfo.gcnArch = 908;
		hipDevInfo.Arch = E_IsaArch::Gfx908;
	}
	void DeviceHip::PrintDeviceDetails()
	{
	}

	StreamHip::StreamHip()
	{
	//	hipStreamCreate(&stream);
	//	hipEventCreate(&startEvt);
	//	hipEventCreate(&stopEvt);

		hipEventCreateWithFlags(&startEvt, hipEventDefault);
		hipEventCreateWithFlags(&stopEvt, hipEventDefault);
	}
	StreamHip * RuntimeHip::CreateStream(bool enProf, int devNum)
	{
		DeviceHip * dev;
		if (devNum < 0)
		{
			dev = (DeviceHip*)selDevice;
		}
		else
		{
			if (devNum >= DevicesCnt())
				return nullptr;

			dev = (DeviceHip*)(devices[devNum]);
		}

		StreamHip * q = new StreamHip();
		dev->AddStream(q);
		return q;

		/*if (q->stream != nullptr)
		{
			dev->AddStream(q);
			return q;
		}
		else
		{
			return nullptr;
		}*/
	}

	KernelHip::KernelHip(char * content, std::string kernelName, E_ProgramType type)
	{
		switch (type)
		{
		case E_ProgramType::CPP_FILE:
		case E_ProgramType::GAS_FILE:
		case E_ProgramType::BIN_FILE:
			programFile = content;
			break;
		case E_ProgramType::CPP_STRING:
		case E_ProgramType::GAS_STRING:
		case E_ProgramType::BIN_ARRAY:
			programSrc = content;
			break;
		}

		programType = type;
		this->kernelName = kernelName;
		kernelFile = RuntimeHip::GetInstance()->KernelTempDir() + DIR_SPT + get_file_name(programFile) + ".o";

		device = (DeviceHip*)(RuntimeHip::GetInstance())->Device();

		createStatus = E_ReturnState::RTN_ERR;
		switch (type)
		{
		case E_ProgramType::CPP_FILE:	createStatus = creatKernelFromCppFile();	break;
		case E_ProgramType::CPP_STRING:	createStatus = creatKernelFromCppString(); break;
		case E_ProgramType::GAS_FILE:	createStatus = creatKernelFromAsmFile();	break;
		case E_ProgramType::GAS_STRING:	createStatus = creatKernelFromAsmString(); break;
		case E_ProgramType::BIN_FILE:	createStatus = creatKernelFromBinFile();	break;
		case E_ProgramType::BIN_ARRAY:	createStatus = creatKernelFromBinArray();	break;
		}
	}
	GpuKernelBase * KernelHip::NewCopy()
	{
		KernelHip * k = new KernelHip();
		k->compiler = compiler;
		k->buildOption = buildOption;
		k->programType = programType;
		k->programFile = programFile;
		k->programSrc = programSrc;
		k->programSize = programSize;
		k->kernelName = kernelName;
		k->kernelFile = kernelFile;
		k->kernelSize = kernelSize;
		k->device = device;
		k->module = module;
		k->kernel = kernel;
		return (GpuKernelBase*)k;
	}
	KernelHip * RuntimeHip::CreateKernel(char * content, std::string kernelName, E_ProgramType type, int devNum)
	{
		DeviceHip * dev;
		if (devNum < 0)
		{
			dev = (DeviceHip*)(selDevice);
		}
		else
		{
			if (devNum >= DevicesCnt())
				return nullptr;

			dev = (DeviceHip*)(devices[devNum]);
		}

		KernelHip * k = new KernelHip(content, kernelName, type);

		if (k->kernel != nullptr)
		{
			if (k->createStatus != E_ReturnState::SUCCESS)
				return nullptr;
			dev->AddKernel(k);
			return k;
		}
		else
		{
			return nullptr;
		}
	}

	E_ReturnState KernelHip::creatKernelFromCppString()
	{
		return E_ReturnState::RTN_ERR;
	}
	E_ReturnState KernelHip::creatKernelFromCppFile()
	{
		return E_ReturnState::SUCCESS;
	}
	E_ReturnState KernelHip::creatKernelFromBinArray()
	{
		return E_ReturnState::RTN_ERR;
	}
	E_ReturnState KernelHip::creatKernelFromBinFile()
	{
		return E_ReturnState::RTN_ERR;
	}
	E_ReturnState KernelHip::creatKernelFromAsmFile()
	{
		//compiler = "/opt/rocm/opencl/bin/x86_64/clang";
		compiler = "/opt/rocm/bin/hcc ";

		switch (device->DeviceInfo().Arch)
		{
		case E_IsaArch::Gfx803:buildOption = "-x assembler -target amdgcn-amd-amdhsa -mcpu=gfx803 -mno-code-object-v3 -c "; break;
		case E_IsaArch::Gfx900:buildOption = "-x assembler -target amdgcn-amd-amdhsa -mcpu=gfx900 -mno-code-object-v3 -c "; break;
		case E_IsaArch::Gfx906:buildOption = "-x assembler -target amdgcn-amd-amdhsa -mcpu=gfx906 -mno-code-object-v3 -c "; break;
		case E_IsaArch::Gfx908:buildOption = "-x assembler -target amdgcn-amd-amdhsa -mcpu=gfx908 -mno-code-object-v3 -c "; break;
		default:ERR("not support hardware.");
		}

		std::string tmpfile = kernelFile + ".tmp";

		std::string cmd = compiler + " " + buildOption + "-o " + tmpfile + " " + programFile;
		exec_cmd(cmd);
		//INFO(cmd);

		cmd = compiler + "-target amdgcn-amd-amdhsa " + tmpfile + " -o " + kernelFile;
		exec_cmd(cmd);

		// creatKernelFromBinString()
		std::ifstream fin(kernelFile.c_str(), std::ios::in | std::ios::binary);
		if (!fin.is_open())
		{
			ERR("can't open bin file: " + kernelFile);
		}

		fin.seekg(0, std::ios::end);
		programSize = (size_t)fin.tellg();
		fin.seekg(0, std::ios::beg);

		programSrc = (char *)malloc(programSize);
		fin.read(programSrc, programSize);
		fin.close();

		// creatKernelFromBinString()
		hipError_t errNum;
		errNum = hipModuleLoad(&module, kernelFile.c_str());
		if (errNum != hipSuccess)
		{
			ERR("Failed to create hip program from " + programFile);
		}

		return buildKernel();
	}
	E_ReturnState KernelHip::creatKernelFromAsmString()
	{
		return E_ReturnState::RTN_ERR;
	}
	E_ReturnState KernelHip::buildKernel()
	{
		hipError_t errNum;

		errNum = hipModuleGetFunction(&kernel, module, kernelName.c_str());
		if (errNum != hipSuccess)
		{
			hipErrInfo(errNum);
			ERR("Failed to build hip program from " + programFile);
		}

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState KernelHip::dumpSource()
	{
		return E_ReturnState::SUCCESS;
	}
	E_ReturnState KernelHip::dumpAssembly()
	{
		return E_ReturnState::SUCCESS;
	}
	E_ReturnState KernelHip::dumpBinary()
	{
		return E_ReturnState::SUCCESS;
	}

	E_ReturnState StreamHip::Launch(GpuKernelBase * kernel, dim global_sz, dim group_sz)
	{
		hipError_t errNum;

		KernelHip * k = (KernelHip*)kernel;

		void * config[] =
		{
			HIP_LAUNCH_PARAM_BUFFER_POINTER, 	k->argsBuff,
			HIP_LAUNCH_PARAM_BUFFER_SIZE,		&k->argsSize,
			HIP_LAUNCH_PARAM_END
		};

		dim grid_sz = global_sz / group_sz;
		
		errNum = hipExtModuleLaunchKernel(
			k->kernel,
			global_sz.x, global_sz.y, global_sz.z,
			group_sz.x, group_sz.y, group_sz.z,
			0, 
			0, 
			NULL,
			(void**)&config,
			startEvt,
			stopEvt
		);
		
		do 
		{
		} while (hipEventQuery(stopEvt) != hipSuccess);		
		
		float elapsedMs = 0;
		hipEventElapsedTime(&elapsedMs, startEvt, stopEvt);
		kernelExeTime = elapsedMs * 1e-3;

		if (errNum != hipSuccess)
		{
			hipErrInfo(errNum);
			ERR("Failed launch kernel: " + std::string(hipGetErrorInfo(errNum)));
		}

		return E_ReturnState::SUCCESS;
	}

	void * RuntimeHip::DevMalloc(size_t byteNum)
	{
		hipError_t errNum;
		void * d_mem;

		errNum = hipMalloc((void**)&d_mem, byteNum);
		if ((errNum != hipSuccess) || (d_mem == nullptr))
		{
			hipErrInfo(errNum);
			d_mem = nullptr;
		}
		return (void*)d_mem;
	}
	void * RuntimeHip::HostMalloc(size_t byteNum, E_MemType memType)
	{
		hipError_t errNum;
		void * h_mem;

		switch (memType)
		{
		case E_MemType::Page:
			h_mem = (float*)malloc(byteNum);
			break;
		case E_MemType::Pin:
			errNum = hipHostMalloc((void**)&h_mem, byteNum, 0);
			break;
		case E_MemType::WrCmb:
			errNum = hipHostMalloc((void**)&h_mem, byteNum, hipHostMallocWriteCombined);
			break;
		case E_MemType::Map:
			errNum = hipHostMalloc((void**)&h_mem, byteNum, hipHostMallocMapped);
			break;
		default:
			h_mem = (float*)malloc(byteNum);
		}

		if ((errNum != hipSuccess) || (h_mem == nullptr))
		{
			hipErrInfo(errNum);
			h_mem = nullptr;
		}
		return (void*)h_mem;
	}
	E_ReturnState RuntimeHip::MemCopyH2D(void * d_mem, void * h_mem, size_t byteNum)
	{
		hipError_t errNum;

		errNum = hipMemcpy(d_mem, h_mem, byteNum, hipMemcpyHostToDevice);
		if (errNum != hipSuccess)
		{
			hipErrInfo(errNum);
			ERR("Failed to copy memory to device %d Byte", byteNum);
		}

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState RuntimeHip::MemCopyD2H(void * h_mem, void * d_mem, size_t byteNum)
	{
		hipError_t errNum;

		errNum = hipMemcpy(h_mem, d_mem, byteNum, hipMemcpyDeviceToHost);
		if (errNum != hipSuccess)
		{
			hipErrInfo(errNum);
			ERR("Failed to copy memory to device %d Byte", byteNum);
		}

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState RuntimeHip::MemCopyD2D(void * d_dst, void * d_src, size_t byteNum)
	{
		hipError_t errNum;

		errNum = hipMemcpy(d_dst, d_src, byteNum, hipMemcpyDeviceToDevice);
		if (errNum != hipSuccess)
		{
			hipErrInfo(errNum);
			ERR("Failed to copy memory to device %d Byte", byteNum);
		}

		return E_ReturnState::SUCCESS;
	}
	E_ReturnState StreamHip::MemCopyH2DAsync(void * d_mem, void * h_mem, size_t byteNum)
	{
		return E_ReturnState::SUCCESS;
	}
	E_ReturnState StreamHip::MemCopyD2HAsync(void * h_mem, void * d_mem, size_t byteNum)
	{
		return E_ReturnState::SUCCESS;
	}
	E_ReturnState StreamHip::MemCopyD2DAsync(void * d_dst, void * d_src, size_t byteNum)
	{
		return E_ReturnState::SUCCESS;
	}

	void RuntimeHip::BandwidthTest()
	{
	}
	void RuntimeHip::bandwidthTest(size_t width, size_t height, E_MemType mem, bool isPitch, BwDir dir, bool isAsync)
	{
	}
}
