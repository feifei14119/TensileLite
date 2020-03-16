import subprocess
import os, re 
import xlwt

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
	
def testOneParam(d, m,n,k, r,s,x,y,u):
	cmd = "./TensileLite.out -m {0} -n {1} -k {2} -l 10 -v 1 -f 2 -t 0 -r {3} -s {4} -x {5} -y {6} -u {7} -d {8}".format(m,n,k,r,s,x,y,u,d)
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
	depthu = [16, 32]
	tile = [1, 2]
	
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
						tflops = testOneParam(d,m,n,k,r,s,x,y,u)
						if(tflops > best_tflops):
							best_r = r
							best_s = s
							best_x = x
							best_y = y
							best_u = u
							best_tflops = tflops;
						
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
						
	print("test for m = {0}, n = {1}, k = {2}: ".format(m,n,k))
	print("best param: r/s = [{0}, {1}]; x/y = [{2}, {3}]; du = {4}.".format(best_r,best_s,best_x,best_y,best_u))
	print("performance = {0} TFLOPs.".format(best_tflops))
	
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
	
def testAll():
	global workbook; 
	global worksheet; 
	global xl_row_cnt;
	workbook = xlwt.Workbook(encoding = 'utf-8');
	
	#d = 1; 	worksheet = workbook.add_sheet('bf32'); xl_row_cnt = 2;
	#m = 960; n = 1024; k = 1024;	testOneSize(d,m,n,k)
	#m = 1024; n = 2048; k = 2048;	testOneSize(d,m,n,k)
	#m = 2048; n = 4096; k = 4096;	testOneSize(d,m,n,k)
	#
	d = 3; 	worksheet = workbook.add_sheet('bf16'); xl_row_cnt = 2;
	m = 960; n = 1024; k = 1024;	testOneSize(d,m,n,k)
	#m = 1024; n = 2048; k = 2048;	testOneSize(d,m,n,k)
	#m = 2048; n = 4096; k = 4096;	testOneSize(d,m,n,k)
	
	workbook.save('perf_test.xls')
	
if __name__ == '__main__': 
	testAll()
	
