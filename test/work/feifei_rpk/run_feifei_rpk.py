# nohup python3 run_feifei_rpk.py > run_feifei_rpk.log 2>&1 &
# jobs
# kill %jobnum
# fg %jobnum
# bg %jobnum

import subprocess
import os, re 
import xlwt
import time
	
def execCmd(cmd):
	r = os.popen(cmd)  
	text = r.read()  
	r.close()  
	return text 
			
def runSgemmRpk(gemm, size, MT):
	cmd = "python3 ../bin/Tensile --cxx-compile hcc ./feifei_rpk/ff_{0}_{1}.yaml ./feifei/{0}_{1}".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	
	os.chdir("./feifei/{0}_{1}/1_BenchmarkProblems/Cijk_Alik_Bljk_SB_00/00_Final/source/assembly/".format(gemm, size, MT))
	cmd = "rm Cijk_Alik_Bljk_SB_*";
	print(cmd);	execCmd(cmd)
	cmd = "cp ../../../../../../../feifei_rpk/Cijk_Alik_Bljk_SB_MT{2}_SE_K1_AutoGen.s ./Cijk_Alik_Bljk_SB_MT{2}_SE_K1.s".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	cmd = "./asm.sh Cijk_Alik_Bljk_SB_MT{2}_SE_K1 -mcpu=gfx908".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	os.chdir("../../build")
	cmd = "rm -rf client CMakeCache.txt CMakeFiles cmake_install.cmake Makefile";
	print(cmd);	execCmd(cmd)
	cmd = "./build.sh";
	print(cmd);	execCmd(cmd)
	cmd = "./run.sh";
	print(cmd);	result = execCmd(cmd);	print(result)
	os.chdir("../../../../../../")
			
def runHgemmRpk(gemm, size, MT):
	cmd = "python3 ../bin/Tensile --cxx-compile hcc ./feifei_rpk/ff_{0}_{1}.yaml ./feifei/{0}_{1}".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	
	os.chdir("./feifei/{0}_{1}/1_BenchmarkProblems/Cijk_Alik_Bljk_HBH_00/00_Final/source/assembly/".format(gemm, size, MT))
	cmd = "rm Cijk_Alik_Bljk_HBH_*";
	print(cmd);	execCmd(cmd)
	cmd = "cp ../../../../../../../feifei_rpk/Cijk_Alik_Bljk_HBH_MT{2}_SE_K1_AutoGen.s ./Cijk_Alik_Bljk_HBH_MT{2}_SE_K1.s".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	cmd = "./asm.sh Cijk_Alik_Bljk_HBH_MT{2}_SE_K1 -mcpu=gfx908".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	os.chdir("../../build")
	cmd = "rm -rf client CMakeCache.txt CMakeFiles cmake_install.cmake Makefile";
	print(cmd);	execCmd(cmd)
	cmd = "./build.sh";
	print(cmd);	execCmd(cmd)
	cmd = "./run.sh";
	print(cmd);	result = execCmd(cmd);	print(result)
	os.chdir("../../../../../../")
			
def runBgemmRpk(gemm, size, MT):
	cmd = "python3 ../bin/Tensile --cxx-compile hcc ./feifei_rpk/ff_{0}_{1}.yaml ./feifei/{0}_{1}".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	
	os.chdir("./feifei/{0}_{1}/1_BenchmarkProblems/Cijk_Alik_Bljk_BBH_00/00_Final/source/assembly/".format(gemm, size, MT))
	cmd = "rm Cijk_Alik_Bljk_BBH_*";
	print(cmd);	execCmd(cmd)
	cmd = "cp ../../../../../../../feifei_rpk/Cijk_Alik_Bljk_BBH_MT{2}_SE_K1_AutoGen.s ./Cijk_Alik_Bljk_BBH_MT{2}_SE_K1.s".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	cmd = "./asm.sh Cijk_Alik_Bljk_BBH_MT{2}_SE_K1 -mcpu=gfx908".format(gemm, size, MT)
	print(cmd);	execCmd(cmd)
	os.chdir("../../build")
	cmd = "rm -rf client CMakeCache.txt CMakeFiles cmake_install.cmake Makefile";
	print(cmd);	execCmd(cmd)
	cmd = "./build.sh";
	print(cmd);	execCmd(cmd)
	cmd = "./run.sh";
	print(cmd);	result = execCmd(cmd);	print(result)
	os.chdir("../../../../../../")
	
def runAll():
	os.chdir("../")
	
	runSgemmRpk("sgemm", "480x1024", "32x128x32");
	runSgemmRpk("sgemm", "480x2048", "32x256x16");
	runSgemmRpk("sgemm", "1024x480", "128x32x16");
	runSgemmRpk("sgemm", "2048x480", "256x32x16");
	
	runHgemmRpk("hgemm", "480x1024", "32x64x32");
	runHgemmRpk("hgemm", "480x2048", "32x256x32");
	runHgemmRpk("hgemm", "1024x480", "128x32x32");
	runHgemmRpk("hgemm", "2048x480", "256x32x32");
	runHgemmRpk("hgemm", "960x1024", "64x128x32");
	
	runBgemmRpk("bgemm", "480x1024", "32x128x32");
	runBgemmRpk("bgemm", "480x2048", "32x256x32");
	runBgemmRpk("bgemm", "1024x480", "128x32x16");
	runBgemmRpk("bgemm", "2048x480", "128x32x32");
	
if __name__ == '__main__': 
	runAll()
	
