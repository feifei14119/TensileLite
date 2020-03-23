#pragma once

#define EN_ROC_FFT		0
#define EN_ROC_RAND		0

// -----------------------------------------------------------------------------
// include path: /opt/rocm/hip/include/
// include file: <hip/hip_runtime.h>
// option 1:
//		compiler: /opt/rocm/bin/hipcc
// 
// option 2:
//		#define __HIP_PLATFORM_HCC__
//		compiler: g++
//		library path: /opt/rocm/lib
//		link lib: hip_hcc
// -----------------------------------------------------------------------------
//#define __HIP_PLATFORM_HCC__
//#include <hip/hip_runtime.h> // aka <hip/hcc_detail/hip_runtime.h>


//#define HIP_PLATFORM 
#include <hip/hip_hcc.h> // aka <hip/hcc_detail/hip_runtime.h>

#if EN_ROC_FFT
// rocfft:
// git clone https://github.com/ROCmSoftwarePlatform/rocFFT.git & git checkout -b master
// ./install -id
// -l rocfft (librocfft.so @ lib)
#include "rocfft.h"
#endif

#if EN_ROC_RAND
// rocrand:
// git clone https://github.com/ROCmSoftwarePlatform/rocRAND.git & git checkout -b master
// mkdir build & cd build
// CXX=hcc cmake ../   或 CXX=hcc cmake -DBUILD_BENCHMARK=ON -BUILD_TEST=ON -BUILD_CRUSH_TEST=ON ../
// make -j16
// sudo make install
// -I /opt/rocm/rocrand/include/ 
// -l rocrand (librocrand.so @ /opt/rocm/rocrand/lib/)
#include "rocrand/rocrand.h"
#endif

namespace feifei 
{
	/************************************************************************/
	/* 硬件信息																*/
	/************************************************************************/
#define	SE_NUM				(4)
#define	CU_PER_SE			(16)
#define CU_NUM				(CU_PER_SE * SE_NUM)
#define	WAVE_SIZE			(64)
#define GPR_SZ				(4)
#define SIMD_PER_CU			(4)
#define	MAX_VGPR_COUNT		(256)
#define	MAX_AGPR_COUNT		(256)
#define	MAX_SGPR_COUNT		(800)
#define MAX_LDS_SIZE		(64*1024)

#define		hipCheckFunc(val)			hip_checkFuncRet((val), #val, __FILE__, __LINE__)
#define		hipCheckErr(val)			hip_checkErrNum((val), __FILE__, __LINE__)
#define		hipErrInfo(val)				hip_printErrInfo((val), __FILE__, __LINE__)

	typedef struct HipDeviceInfoType : T_DeviceInfo
	{
		int major;
		int minor;

		int maxThreadsPerBlock;     /**< Maximum number of threads per block */
		int maxThreadsDim[3];       /**< Maximum size of each dimension of a block */
		int maxGridSize[3];         /**< Maximum size of each dimension of a grid */

		int enGlobalL1;
		int enLocalL1;
		int regsPerBlock;           /**< 32-bit registers available per block */
		int totalConstantMemory;    /**< Constant memory available on device in bytes */
		int maxPitchMem;               /**< Maximum pitch in bytes allowed by memory copies */

		int clockInstructionRate; /*< Frequency in khz of the timer used by the device - side "clock*instructions" >*/
		int concurrentKernels;		/*< Device can possibly execute multiple kernels concurrently.*/
		int pciDomainID;           //< PCI Domain ID
		int pciBusID;              //< PCI Bus ID.
		int pciDeviceID;           //< PCI Device ID.
		size_t maxSharedMemoryPerMultiProcessor;
		int isMultiGpuBoard;                      ///< 1 if device is on a multi-GPU board, 0 if not.
		int canMapHostMemory;                     ///< Check whether HIP can map host memory
		int gcnArch;                              ///< AMD GCN Arch Value. Eg: 803, 701
		int integrated;            ///< APU vs dGPU
		int computeMode;                  ///< Compute mode.
	} T_hipDeviceInfo;

	static const char *hipGetErrorInfo(hipError_t error)
	{
		switch (error)
		{
		case hipSuccess:return "hipSuccess";
		//case hipErrorOutOfMemory:return "hipErrorOutOfMemory";
		//case hipErrorNotInitialized:return "hipErrorNotInitialized";
		case hipErrorDeinitialized:return "hipErrorDeinitialized";
		case hipErrorProfilerDisabled:return "hipErrorProfilerDisabled";
		case hipErrorProfilerNotInitialized:return "hipErrorProfilerNotInitialized";
		case hipErrorProfilerAlreadyStarted:return "hipErrorProfilerAlreadyStarted";
		case hipErrorProfilerAlreadyStopped:return "hipErrorProfilerAlreadyStopped";
		case hipErrorInsufficientDriver:return "hipErrorInsufficientDriver";
		case hipErrorInvalidImage:return "hipErrorInvalidImage";
		case hipErrorInvalidContext:return "hipErrorInvalidContext";
		case hipErrorContextAlreadyCurrent:return "hipErrorContextAlreadyCurrent";
		//case hipErrorMapFailed:return "hipErrorMapFailed";
		case hipErrorUnmapFailed:return "hipErrorUnmapFailed";
		case hipErrorArrayIsMapped:return "hipErrorArrayIsMapped";
		case hipErrorAlreadyMapped:return "hipErrorAlreadyMapped";
		case hipErrorNoBinaryForGpu:return "hipErrorNoBinaryForGpu";
		case hipErrorAlreadyAcquired:return "hipErrorAlreadyAcquired";
		case hipErrorNotMapped:return "hipErrorNotMapped";
		case hipErrorNotMappedAsArray:return "hipErrorNotMappedAsArray";
		case hipErrorNotMappedAsPointer:return "hipErrorNotMappedAsPointer";
		case hipErrorECCNotCorrectable:return "hipErrorECCNotCorrectable";
		case hipErrorUnsupportedLimit:return "hipErrorUnsupportedLimit";
		case hipErrorContextAlreadyInUse:return "hipErrorContextAlreadyInUse";
		case hipErrorPeerAccessUnsupported:return "hipErrorPeerAccessUnsupported";
		case hipErrorInvalidKernelFile:return "hipErrorInvalidKernelFile";
		case hipErrorInvalidGraphicsContext:return "hipErrorInvalidGraphicsContext";
		case hipErrorInvalidSource:return "hipErrorInvalidSource";
		case hipErrorFileNotFound:return "hipErrorFileNotFound";
		case hipErrorSharedObjectSymbolNotFound:return "hipErrorSharedObjectSymbolNotFound";
		case hipErrorSharedObjectInitFailed:return "hipErrorSharedObjectInitFailed";
		case hipErrorOperatingSystem:return "hipErrorOperatingSystem";
		case hipErrorSetOnActiveProcess:return "hipErrorSetOnActiveProcess";
		//case hipErrorInvalidHandle:return "hipErrorInvalidHandle";
		case hipErrorNotFound:return "hipErrorNotFound";
		case hipErrorIllegalAddress:return "hipErrorIllegalAddress";
		case hipErrorInvalidSymbol:return "hipErrorInvalidSymbol";
		case hipErrorMissingConfiguration:return "hipErrorMissingConfiguration";
		//case hipErrorMemoryAllocation:return "hipErrorMemoryAllocation";
		//case hipErrorInitializationError:return "hipErrorInitializationError";
		case hipErrorLaunchFailure:return "hipErrorLaunchFailure";
		case hipErrorLaunchTimeOut:return "hipErrorLaunchTimeOut";
		case hipErrorLaunchOutOfResources:return "hipErrorLaunchOutOfResources";
		case hipErrorInvalidDeviceFunction:return "hipErrorInvalidDeviceFunction";
		case hipErrorInvalidConfiguration:return "hipErrorInvalidConfiguration";
		case hipErrorInvalidDevice:return "hipErrorInvalidDevice";
		case hipErrorInvalidValue:return "hipErrorInvalidValue";
		case hipErrorInvalidDevicePointer:return "hipErrorInvalidDevicePointer";
		case hipErrorInvalidMemcpyDirection:return "hipErrorInvalidMemcpyDirection";
		case hipErrorUnknown:return "hipErrorUnknown";
		//case hipErrorInvalidResourceHandle:return "hipErrorInvalidResourceHandle";
		case hipErrorNotReady:return "hipErrorNotReady";
		case hipErrorNoDevice:return "hipErrorNoDevice";
		case hipErrorPeerAccessAlreadyEnabled:return "hipErrorPeerAccessAlreadyEnabled";
		case hipErrorPeerAccessNotEnabled:return "hipErrorPeerAccessNotEnabled";
		case hipErrorRuntimeMemory:return "hipErrorRuntimeMemory";
		case hipErrorRuntimeOther:return "hipErrorRuntimeOther";
		case hipErrorHostMemoryAlreadyRegistered:return "hipErrorHostMemoryAlreadyRegistered";
		case hipErrorHostMemoryNotRegistered:return "hipErrorHostMemoryNotRegistered";
		//case hipErrorMapBufferObjectFailed:return "hipErrorMapBufferObjectFailed";
		case hipErrorAssert:return "hipErrorAssert";
		case hipErrorTbd:return "hipErrorTbd";
		}

		return "<unknown>";
	}
	static void hip_checkFuncRet(hipError_t errNum, char const *const func, const char *const file, int const line)
	{
		if (errNum)
		{
			fprintf(stderr, "Hip error at %s:%d code=%d(%s) \"%s\" \n", file, line, static_cast<unsigned int>(errNum), hipGetErrorInfo(errNum), func);

			exit(EXIT_FAILURE);
		}
	}
	static void hip_checkErrNum(hipError_t errNum, const char *const file, int const line)
	{
		if (errNum)
		{
			fprintf(stderr, "Hip error at %s:%d code=%d(%s) \n", file, line, static_cast<unsigned int>(errNum), hipGetErrorInfo(errNum));

			exit(EXIT_FAILURE);
		}
	}
	static void hip_printErrInfo(hipError_t errNum, const char *const file, int const line)
	{
		if (errNum)
		{
			fprintf(stderr, "Hip error at %s:%d code=%d(%s) \n", file, line, static_cast<unsigned int>(errNum), hipGetErrorInfo(errNum));
		}
	}

	class StreamHip;
	class KernelHip;
	class RuntimeHip;

	class DeviceHip:public GpuDeviceBase
	{
	protected:
		friend class RuntimeHip;
		friend class StreamHip;
		friend class KernelHip;

	protected:
		E_ReturnState initDevice() { return E_ReturnState::SUCCESS; }
		void deInitDevice() { ; }

		T_hipDeviceInfo hipDevInfo;
		void getDeviceInfo();

	public:
		DeviceHip(uint32_t id) : GpuDeviceBase((uint64_t)id)
		{
			deviceInfo = (T_DeviceInfo*)(&hipDevInfo);
			getDeviceInfo();
		}

		void PrintDeviceDetails();

		void Sync() { hipDeviceSynchronize(); }
	}; 
	
	class StreamHip:public GpuStreamBase
	{
	protected:
		friend class KernelHip;
		friend class RuntimeHip;

	public:
		StreamHip();
		~StreamHip()
		{
			hipEventDestroy(startEvt);
			hipEventDestroy(stopEvt);

			hipStreamDestroy(stream);
		}

		E_ReturnState MemCopyH2DAsync(void * d_mem, void * h_mem, size_t byteNum);
		E_ReturnState MemCopyD2HAsync(void * h_mem, void * d_mem, size_t byteNum);
		E_ReturnState MemCopyD2DAsync(void * d_dst, void * d_src, size_t byteNum);

		E_ReturnState Launch(GpuKernelBase *k, dim global_sz, dim group_sz);
		void Flush() { hipStreamSynchronize(stream); }

	private:
		DeviceHip * device;
		hipStream_t stream;
		hipEvent_t startEvt, stopEvt;
	};

	class KernelHip:public GpuKernelBase
	{
	protected:
		friend class StreamHip;
		friend class RuntimeHip;

	public:
		KernelHip() {}
		KernelHip(char * content, std::string kernelName, E_ProgramType type);
		GpuKernelBase * NewCopy();

	protected:
		DeviceHip * device;
		hipModule_t module;		// 程序对象
		hipFunction_t kernel;	// kernel对象
		
		E_ReturnState creatKernelFromCppFile();
		E_ReturnState creatKernelFromCppString();
		E_ReturnState creatKernelFromAsmFile();
		E_ReturnState creatKernelFromAsmString();
		E_ReturnState creatKernelFromBinFile();
		E_ReturnState creatKernelFromBinArray();
		E_ReturnState buildKernel();
		E_ReturnState dumpSource();
		E_ReturnState dumpAssembly();
		E_ReturnState dumpBinary();
	};

	class RuntimeHip:public GpuRuntimeBase
	{
	protected:
		friend class KernelHip;
		friend class StreamHip;

	private:
		static RuntimeHip * pInstance;
		enum class BwDir
		{
			H2D = 1,
			D2H = 2,
			D2D = 3,
			H2H = 4
		};

	protected:
		E_ReturnState initPlatform();
		void getPlatformInfo();
		void bandwidthTest(size_t width, size_t height, E_MemType mem, bool isPitch, BwDir dir, bool isAsync);

	public:
		static RuntimeHip * GetInstance();
		~RuntimeHip()
		{
			for (unsigned int i = 0; i < DevicesCnt(); i++)
			{
				DeviceHip * pDev = (DeviceHip*)(devices[i]);
				pDev->deInitDevice();
			}
			
			if (RuntimeHip::pInstance)
				pInstance = nullptr;
		}

		StreamHip * CreateStream(bool enProf = false, int devNum = -1);
		KernelHip * CreateKernel(char * content, std::string kernelName, E_ProgramType type, int devNum = -1);

		void * DevMalloc(size_t byteNum);
		void * HostMalloc(size_t byteNum, E_MemType memType = E_MemType::Page);
		void DevFree(void * d_mem) { hipFree(d_mem); }
		void HostFree(void * h_mem, E_MemType memType = E_MemType::Page)
		{
			if (memType == E_MemType::Page)
			{
				free(h_mem);
			}
			else
			{
				hipHostFree(h_mem);
			}
		}
		E_ReturnState MemCopyH2D(void * d_mem, void * h_mem, size_t byteNum);
		E_ReturnState MemCopyD2H(void * h_mem, void * d_mem, size_t byteNum);
		E_ReturnState MemCopyD2D(void * d_dst, void * d_src, size_t byteNum);

		void BandwidthTest();
	};
}
