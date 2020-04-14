# nohup python3 perf_test.py > test_log.txt 2>&1 &
# jobs
# kill %1
import subprocess
import os, re 
import xlwt
import time

workbook = 0;
worksheet = 0;
xl_row_cnt = 0;
buffer_num = 2;
	
	
def execCmd(cmd):  
	r = os.popen(cmd)  
	text = r.read()  
	r.close()  
	return text 

def writeFile(filename, data):  
	f = open(filename, "w+")  
	f.write(data)  
	f.close()
	
def testOneParam(d, m,n,k, r,s,x,y,u):
	global buffer_num; 
	cmd = "../TensileLite.out -m {0} -n {1} -k {2} -l 100 -v 0 -f {9} -t 0 -r {3} -s {4} -x {5} -y {6} -u {7} -d {8}".format(m,n,k,r,s,x,y,u,d,buffer_num)
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
	best_tflops = float(0);	
	
	tflops = float(0)
	depthu = [16, 32, 64, 128]
	tile = [1, 2, 4]
	
	global worksheet; 
	global xl_row_cnt;
	xl_col_cnt = 2;
	xl_row_cnt = xl_row_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'm');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'n');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'k');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'r');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 's');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'x');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'y');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'u');  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'tflops');  xl_col_cnt = xl_col_cnt + 1;
	
	for r in tile:
		for s in tile:
			for x in tile:
				for y in tile:
					for u in depthu:
						tflops = float(0)
						tflops = testOneParam(d,m,n,k,r,s,x,y,u)
						#time.sleep(0.1)
						if(tflops > best_tflops):
							best_r = r
							best_s = s
							best_x = x
							best_y = y
							best_u = u
							best_tflops = tflops;
						
						if(tflops > 0):
							xl_row_cnt = xl_row_cnt + 1;
							xl_col_cnt = 2;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = m);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = n);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = k);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = r);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = s);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = x);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = y);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = u);  xl_col_cnt = xl_col_cnt + 1;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = tflops);  xl_col_cnt = xl_col_cnt + 1;
							
							theory = float(0)
							if(d == 1): theory = 1289*120*256/1000/1000;
							if(d == 2): theory = 1289*120*1024/1000/1000;
							if(d == 3): theory = 1289*120*512/1000/1000;
							efficiency = tflops / theory;
							worksheet.write(xl_row_cnt, xl_col_cnt, label = efficiency);  xl_col_cnt = xl_col_cnt + 1;
	
	theory = float(0)
	if(d == 1): theory = 1289*120*256/1000/1000;
	if(d == 2): theory = 1289*120*1024/1000/1000;
	if(d == 3): theory = 1289*120*512/1000/1000;
	efficiency = best_tflops / theory;
						
	print("test for m = {0}, n = {1}, k = {2}: ".format(m,n,k))
	print("best param: r/s = [{0}, {1}]; x/y = [{2}, {3}]; du = {4}.".format(best_r,best_s,best_x,best_y,best_u))
	print("performance = {0} TFLOPs.".format(best_tflops))
	print("efficiency = {0} %.".format(efficiency * 100.0))
	
	xl_row_cnt = xl_row_cnt + 1;	xl_col_cnt = 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = 'fast');  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = m);  		xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = n);  		xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = k);  		xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_r);  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_s);  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_x);  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_y);  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_u);  	xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = best_tflops);  xl_col_cnt = xl_col_cnt + 1;
	worksheet.write(xl_row_cnt, xl_col_cnt, label = efficiency);  xl_col_cnt = xl_col_cnt + 1;
	
def testAll():
	global workbook; 
	global worksheet; 
	global xl_row_cnt;
	global buffer_num; 
	workbook = xlwt.Workbook(encoding = 'utf-8');
	
	cmd = 'sudo ~/umr -i 1 -w arcturus.gfx90.mmSQ_CONFIG 0x01180000';execCmd(cmd)
	buffer_num = 2;
	d = 1; 	worksheet = workbook.add_sheet('fp32'); xl_row_cnt = 2;
	m = 960; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	m = 1920; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	m = 3840; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	m = 7680; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	
	d = 3; 	worksheet = workbook.add_sheet('bf16'); xl_row_cnt = 2;
	m = 960; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	m = 1920; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	m = 3840; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	m = 7680; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	
	#cmd = 'sudo ~/umr -i 1 -w arcturus.gfx90.mmSQ_CONFIG 0x01180001';execCmd(cmd)
	#buffer_num = 3;
	#d = 2; 	worksheet = workbook.add_sheet('fp16'); xl_row_cnt = 2;
	#m = 960; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	#m = 1920; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	#m = 3840; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	#m = 7680; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('gemm_perf.xls')
	
	workbook.save('gemm_perf.xls')
	
	# #####################################################################################################
	# #####################################################################################################
	
	cmd = 'sudo ~/umr -i 1 -w arcturus.gfx90.mmSQ_CONFIG 0x01180000';execCmd(cmd)
	buffer_num = 2;
	d = 1; 	worksheet = workbook.add_sheet('fp32'); xl_row_cnt = 2;
	m = 480; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512;  n = 480; k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 1024; n = 480; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 2048; n = 480; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 4096; n = 480; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 8192; n = 480; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512;  n = 512; k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 1024; n = 512; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 2048; n = 512; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 4096; n = 512; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 8192; n = 512; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	d = 3; 	worksheet = workbook.add_sheet('bf16'); xl_row_cnt = 2;
	m = 480; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512;  n = 480; k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 1024; n = 480; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 2048; n = 480; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 4096; n = 480; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 8192; n = 480; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512;  n = 512; k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 1024; n = 512; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 2048; n = 512; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 4096; n = 512; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 8192; n = 512; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	cmd = 'sudo ~/umr -i 1 -w arcturus.gfx90.mmSQ_CONFIG 0x01180001';execCmd(cmd)
	buffer_num = 3;
	d = 2; 	worksheet = workbook.add_sheet('fp16'); xl_row_cnt = 2;
	m = 480; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 480; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512; n = 512;  k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 1024; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 2048; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 4096; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 512; n = 8192; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512;  n = 480; k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 1024; n = 480; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 2048; n = 480; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 4096; n = 480; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 8192; n = 480; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	m = 512;  n = 512; k = 512;		testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 1024; n = 512; k = 1024;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 2048; n = 512; k = 2048;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 4096; n = 512; k = 4096;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	m = 8192; n = 512; k = 8192;	testOneSize(d,m,n,k);	workbook.save('phantom_perf.xls')
	
	workbook.save('phantom_perf.xls')
	
if __name__ == '__main__': 
	testAll()
	
