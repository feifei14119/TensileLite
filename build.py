import subprocess
import sys, os, re
import shutil

def execCmd(cmd):  
	r = os.popen(cmd)  
	text = r.read() 
	print(text)
	r.close()  
	return text 
	

def buildLibGpu():
	if os.path.exists("../lib/libff_gpu.a"):
		print("remove ../lib/libff_gpu.a")
		os.remove("../lib/libff_gpu.a")
		
	cmd = 'hipcc ../ff_gpu_src/ff_gpu.cpp ../ff_gpu_src/ff_hip_runtime.cpp ../ff_gpu_src/ff_gpu_test_fw.cpp -D"__GPU_RT_HIP"  -c -fPIC -O0 -w -std=c++11 -I"/opt/rocm/include/"'
	print(cmd)
	execCmd(cmd)
	
	cmd = 'ar cr ../lib/libff_gpu.a ff_gpu.o ff_hip_runtime.o ff_gpu_test_fw.o'
	print(cmd)
	execCmd(cmd)
	
def buildApp():
	if os.path.exists("../TensileLite.out"):
		print("remove ../TensileLite.out")
		os.remove("../TensileLite.out")
		
	cmd = 'hipcc ../src/gemmMfma.cpp ../src/main.cpp -c -fPIC -D"__GPU_RT_HIP" -O0 -w -std=c++11 -I"/opt/rocm/include/"'
	print(cmd)
	execCmd(cmd)
	
	cmd = 'g++ -o ../TensileLite.out ./gemmMfma.o ./main.o ../lib/libff_gpu.a ../lib/libff_utils.a /opt/rocm/lib/libhip_hcc.so'
	print(cmd)
	execCmd(cmd)
	
def testApp():	
	cmd = '../TensileLite.out -m 960 -n 1024 -k 1024 -v 1 -l 10 -f 2 -r 1 -s 2 -x 2 -y 2 -z 32 -u 32 -d 3'
	print(cmd)
	execCmd(cmd)
	
if __name__ == '__main__':
	if(os.path.exists("./build")):
		shutil.rmtree("./build")
	os.mkdir("./build")
	os.chdir("./build")

	if(len(sys.argv) == 1):
		buildLibGpu()
		buildApp()
		testApp()
		exit()
	
	arg = sys.argv[1]
	
	if(arg == '1'):
		buildLibGpu()
		exit()
	
	if(arg == '2'):
		buildApp()
		testApp()
		exit()
		
	if(arg == '3'):
		testApp()
		exit()
	
	
