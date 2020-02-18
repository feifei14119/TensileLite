#include "../inc/ff_utils.h"
#include "../inc/ff_gpu.h"
#include "HgemmMfma.h"

using namespace feifei;

int main(int argc, char *argv[])
{
	CmdArgs * ca = new CmdArgs(argc, argv);
	
	LOG("Current Path: " + get_curr_path());
	LOG("Work Path: " + get_work_path());
	LOG("Kernel Path: " + get_kernel_path());

	GpuRuntimeBase * pGpuRt = GpuRuntime::GetInstance();

	pGpuRt->PrintRuntimeInfo(true);
	pGpuRt->SellectDevice(0);
	pGpuRt->Device()->PrintDeviceInfo();

	TEST(GemmMfmaProblem);
	RunGpuExample();

	delete pGpuRt;	
	return 0;
}
