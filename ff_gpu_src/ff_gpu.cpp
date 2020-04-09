#include "../inc/ff_gpu.h"

namespace feifei
{
	static std::string g_KernelPath = "";
	void init_kernel_path()
	{
		g_KernelPath = get_work_path() + "kernel" + DIR_SPT;
	}
	std::string get_kernel_path()
	{
		if (g_KernelPath == "")
			init_kernel_path();
		return g_KernelPath;
	}
	void set_kernel_path(std::string path)
	{
		g_KernelPath = path;
	}
	   
	E_GpuRt GpuRuntime::GpuRtType() 
	{
		return GPU_RUNTIME_TEMP; 
	}
	GpuRuntimeBase * GpuRuntime::GetInstance()
	{
		switch (GpuRuntime::GpuRtType())
		{
#ifdef __GPU_RT_OCL
		case E_GpuRt::OCL: return RuntimeOcl::GetInstance();
#endif
#ifdef __GPU_RT_CU_DRV
		case E_GpuRt::CU_DRV: return RuntimeCuDrv::GetInstance();
#endif
#ifdef __GPU_RT_HSA
		case E_GpuRt::HSA: return RuntimeHsa::GetInstance();
#endif
#ifdef __GPU_RT_HIP
		case E_GpuRt::HIP: return RuntimeHip::GetInstance();
#endif
		default:return NULL;
		}
	}
	GpuRuntimeBase * GpuRuntime::GetInstance(E_GpuRt rtType)
	{
		switch (rtType)
		{
#ifdef __GPU_RT_OCL
		case E_GpuRt::OCL: return RuntimeOcl::GetInstance();
#endif
#ifdef __GPU_RT_CU_DRV
		case E_GpuRt::CU_DRV: return RuntimeCuDrv::GetInstance();
#endif
#ifdef __GPU_RT_HSA
		case E_GpuRt::HSA: return RuntimeHsa::GetInstance();
#endif
#ifdef __GPU_RT_HIP
		case E_GpuRt::HIP: return RuntimeHip::GetInstance();
#endif
		default:return NULL;
		}
	}

	DispatchType::DispatchType()
	{
		this->stream = nullptr;
		this->kernel = nullptr;
		this->group_size = dim(0, 0, 0);
		this->global_size = dim(0, 0, 0);
	}
	DispatchType::DispatchType(GpuStreamBase * stream, dim global_size, dim group_size)
	{
		if (stream == nullptr)
		{
			GpuRuntimeBase * rt = GpuRuntime::GetInstance();
			this->stream = rt->GetDefaultStream();
		}
		else
		{
			this->stream = stream;
		}
		this->group_size = group_size;
		this->global_size.x = ((global_size.x + group_size.x - 1) / group_size.x) * group_size.x;
		this->global_size.y = ((global_size.y + group_size.y - 1) / group_size.y) * group_size.y;
		this->global_size.z = ((global_size.z + group_size.z - 1) / group_size.z) * group_size.z;
	}
	DispatchType::DispatchType(dim global_size, dim group_size)
	{
		GpuRuntimeBase * rt = GpuRuntime::GetInstance();
		this->stream = rt->GetDefaultStream();
		this->group_size = group_size;
		this->global_size.x = ((global_size.x + group_size.x - 1) / group_size.x) * group_size.x;
		this->global_size.y = ((global_size.y + group_size.y - 1) / group_size.y) * group_size.y;
		this->global_size.z = ((global_size.z + group_size.z - 1) / group_size.z) * group_size.z;
	}

	void DataMemTest()
	{
		float * a = (float*)malloc(1024 * sizeof(float));
		for (int i = 0; i < 1024; i++)
			a[i] = 1.0f*i;
		DataMem<float> * mem = new DataMem<float>("data-a", a, E_MemType::Page, 512, 2);
		printf("mem_size = %zd.\n", mem->MemSize());
		mem->DumpBin();
		mem->LoadBin("data-a.bin", 512);
		mem->Log(E_DataFormat::Nomal, 1, 0, 31);
		mem->Dump(E_DataFormat::Nomal, 2);
	}
	//void CompareDataMem(DataMem<cplx_fp32> * rslt, DataMem<cplx_fp32> * ref, uint64_t verify_len)	{}
	DataMem<float> * InitRealData(std::string name, float init_type, uint32_t dim0, uint32_t dim1, uint32_t dim2)
	{
		float * h_x = (float*)malloc(dim0*dim1*dim2 * sizeof(float));
		if (init_type != 0)
		{
			for (uint32_t bt = 0; bt < dim2; bt++)
			{
				for (uint32_t ch = 0; ch < dim1; ch++)
				{
					float * _x = h_x + dim0 * (dim1 * bt + ch);

					for (uint32_t i = 0; i < dim0; i++)
					{
						uint32_t idx = dim0 * (dim1 * bt + ch) + i;

						if (init_type == 0.5f) { _x[i] = 0; }
						else if (init_type == 1.0f) { _x[i] = 1.0f; }
						else if (init_type == 2.0f) { _x[i] = 1.0f * idx; }
						else if (init_type == 2.1f) { _x[i] = 0.01f * (idx % 1137); }
						else if (init_type == 2.2f) { _x[i] = (i + 1) * 0.01f + ch * 1.0f + bt * 100.0f; }
						else if (init_type == 2.3f) { _x[i] = 0.1f * (idx % 17); }
						else if (init_type == 2.4f) { _x[i] = 1.0f * (idx % 7); }
						else if (init_type == -1.0f) { _x[i] = -1.0f; }
						else if (init_type == -2.0f) { _x[i] = -1.0f * idx; }
						else if (init_type == -2.1f) { _x[i] = -0.01f * (idx % 1137); }
						else if (init_type == -2.2f) { _x[i] = (i + 1) * -0.01f + ch * -1.0f + bt * -100.0f; }
						else if (init_type == -2.3f) { _x[i] = -0.1f * (idx % 17); }
						else if (init_type == -2.4f) { _x[i] = -1.0f * (idx % 11); }
						else { _x[i] = init_type; }
					}
				}
			}
		}

		DataMem<float> * dm = new DataMem<float>(name, h_x, E_MemType::Page, dim0, dim1, dim2);
		dm->Sync2Dev();
		dm->SetMemType(E_MemType::Dev);
		return dm;
	}
	//DataMem<cplx_fp32> * InitCplxData(std::string name, uint32_t dim0, uint32_t dim1, uint32_t dim2)	{}
}
