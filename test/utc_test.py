# nohup python3 perf_test.py > test.log 2>&1 &

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

def writeFile(filename, data):  
	f = open(filename, "w+")  
	f.write(data)  
	f.close()
	
def testOneParam(d, m,n,k, r,s,x,y,z,u,f,p):
	cmd = "../TensileLite.out -m {0} -n {1} -k {2} -l 100 -v 0 -f {10} -t 0 -r {3} -s {4} -x {5} -y {6} -z {7} -u {8} -d {9} -p {11}".format(m,n,k,r,s,x,y,z,u,d,f,p)
	print(cmd)
	result = execCmd(cmd)
	result = re.split("\n", result)
	tflops = float(0)
	for line in result:
		idx1 = line.rfind("]Performance = ")
		idx2 = line.rfind("TFLOPs")
		if(idx1 != -1):
			print(line)
			line = line[idx1 + len("]Performance = "):idx2-1]
			tflops = float(line)
	return tflops
			
def testOneSize(d, m,n,k):
	best_r = 0
	best_s = 0
	best_x = 0
	best_y = 0
	best_u = 0
	best_p = 0
	best_stride = 0
	best_tflops = float(0);
	sclk = float(1289);
	sclk = float(878);
	sclk = float(1189);
	
	tflops = float(0)
	
	pad_num = [512, 1024, 1024*2, 1024*4, 1024*16, 
				1024*20, 1024*25, 1024*30, 1024*35, 
				1024*40, 1024*50, 1024*60, 1024*70,
				1024*80, 1024*85, 1024*90, 1024*100,
				1024*120, 1024*140, 1024*160, 1024*180, 1024*200]
	
	#pad_num = [512, 1024]
	for i in range(0,len(pad_num)):
		pad_num[i] = pad_num[i] - 512 + 32;
	
	global worksheet; 
	global xl_row_cnt;
	xl_col_cnt = 2;
	xl_row_cnt = xl_row_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'pad');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'tflops');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'efficiency');  xl_col_cnt = xl_col_cnt + 1;
	
	for p in pad_num:
		k=r=s=x=y=z=u=f=1;
		tflops = float(0)
		tflops = testOneParam(d,m,n,k,r,s,x,y,z,u,f,p)
		#time.sleep(0.1)
		if(tflops > best_tflops):
			best_stride = p + 512 - 32
			best_tflops = tflops;
								
		if(tflops > 0):
			stride = p + 512 - 32
			xl_row_cnt = xl_row_cnt + 1;
			xl_col_cnt = 2;
			worksheet.write(xl_row_cnt, xl_col_cnt, label = stride);  xl_col_cnt = xl_col_cnt + 1;
			worksheet.write(xl_row_cnt, xl_col_cnt, label = tflops);  xl_col_cnt = xl_col_cnt + 1;
			
			theory = float(0)
			if(d == 1): theory = sclk*120*256/1000/1000;
			if(d == 2): theory = sclk*120*1024/1000/1000;
			if(d == 3): theory = sclk*120*512/1000/1000;
			efficiency = tflops / theory;
			worksheet.write(xl_row_cnt, xl_col_cnt, label = efficiency);  xl_col_cnt = xl_col_cnt + 1;
	
	theory = float(0)
	if(d == 1): theory = sclk*120*256/1000/1000;
	if(d == 2): theory = sclk*120*1024/1000/1000;
	if(d == 3): theory = sclk*120*512/1000/1000;
	efficiency = best_tflops / theory;
						
	print("test for m = {0}, n = {1}, k = {2}: ".format(m,n,k))
	print("best param: pad = {0}.".format(best_stride))
	print("performance = {0} TFLOPs.".format(best_tflops))
	print("efficiency = {0} %.".format(efficiency * 100.0))
	
	xl_row_cnt = xl_row_cnt + 1;	xl_col_cnt = 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'fast');  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_stride);  		xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_tflops);  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = efficiency);  xl_col_cnt = xl_col_cnt + 1;
	
def testAll():
	global workbook; 
	global worksheet; 
	global xl_row_cnt;
	workbook = xlwt.Workbook(encoding = 'utf-8');
	
	d = 2; 	worksheet = workbook.add_sheet('fp16'); xl_row_cnt = 2;	
	m = 480; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('with_preload_perf.xls')
	workbook.save('with_preload_perf.xls')
	#m = 480; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('without_preload_perf.xls')
	#workbook.save('without_preload_perf.xls')
	
if __name__ == '__main__': 
	testAll()
	
