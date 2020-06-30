# nohup python3 gen_tensile_rpk.py > gen_tensile_rpk.log 2>&1 &
# jobs
# kill %jobnum
# fg %jobnum
# bg %jobnum

import subprocess
import os, re 
import xlwt
import time

workbook = 0;
worksheet = 0;
xl_row_cnt = 0;
	
	
def execCmd(cmd):  
	r = os.popen(cmd)  
	text = r.read()  
	r.close()  
	return text 
			
def genOneRpk(gemm, size):
	cmd = "python3 ../bin/Tensile --cxx-compile hcc ./yaml/ts_{0}_{1}.yaml ./tensile/{0}_{1}".format(gemm, size)
	print(cmd)
	result = execCmd(cmd)
	print(result)
	
	if(gemm == "bgemm")
		return;
	cmd = "yes|cp -fr ./tensile/{0}_{1}/1_BenchmarkProblems/Cijk_Alik_Bljk_*/00_Final/source/assembly/Cijk_Alik_Bljk_* ./tensile_rpk".format(gemm, size)
	print(cmd)
	result = execCmd(cmd)
	print(result)
	
def genAll():
	os.chdir("../")
	
	genOneRpk("sgemm", "480x1024");
	genOneRpk("sgemm", "480x2048");
	genOneRpk("sgemm", "1024x480");
	genOneRpk("sgemm", "2048x480");
	
	genOneRpk("hgemm", "480x1024");
	genOneRpk("hgemm", "480x2048");
	genOneRpk("hgemm", "960x1024");
	genOneRpk("hgemm", "1024x480");
	genOneRpk("hgemm", "2048x480");
	
	genOneRpk("bgemm", "480x1024");
	genOneRpk("bgemm", "480x2048");
	genOneRpk("bgemm", "1024x480");
	genOneRpk("bgemm", "2048x480");
	
if __name__ == '__main__': 
	genAll()
	
