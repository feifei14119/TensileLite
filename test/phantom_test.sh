#!/bin/bash
# ./phantom_test.sh > phantom_log.txt 2>&1


./TensileLite.out -d 1 -m  512 -n  512 -k  512 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  512 -n 1024 -k 1024 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  512 -n 2048 -k 2048 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  512 -n 4096 -k 4096 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  512 -n 8192 -k 8192 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2

./TensileLite.out -d 1 -m  512 -n  512 -k  512 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 1024 -n  512 -k 1024 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 2048 -n  512 -k 2048 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 4096 -n  512 -k 4096 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 8192 -n  512 -k 8192 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2

./TensileLite.out -d 1 -m  480 -n  512 -k  512 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  480 -n 1024 -k 1024 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  480 -n 2048 -k 2048 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  480 -n 4096 -k 4096 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m  480 -n 8192 -k 8192 -r 1 -s 1 -x 1 -y 4 -u 32 -v 0 -l 1 -f 2

./TensileLite.out -d 1 -m  512 -n  480 -k  512 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 1024 -n  480 -k 1024 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 2048 -n  480 -k 2048 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 4096 -n  480 -k 4096 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2
./TensileLite.out -d 1 -m 8192 -n  480 -k 8192 -r 1 -s 1 -x 4 -y 1 -u 32 -v 0 -l 1 -f 2

##################################################################################

./TensileLite.out -d 3 -m  512 -n  512 -k  512 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  512 -n 1024 -k 1024 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  512 -n 2048 -k 2048 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  512 -n 4096 -k 4096 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  512 -n 8192 -k 8192 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2

./TensileLite.out -d 3 -m  512 -n  512 -k  512 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 1024 -n  512 -k 1024 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 2048 -n  512 -k 2048 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 4096 -n  512 -k 4096 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 8192 -n  512 -k 8192 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2

./TensileLite.out -d 3 -m  480 -n  512 -k  512 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  480 -n 1024 -k 1024 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  480 -n 2048 -k 2048 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  480 -n 4096 -k 4096 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m  480 -n 8192 -k 8192 -r 1 -s 1 -x 1 -y 4 -u 64 -v 0 -l 1 -f 2

./TensileLite.out -d 3 -m  512 -n  480 -k  512 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 1024 -n  480 -k 1024 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 2048 -n  480 -k 2048 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 4096 -n  480 -k 4096 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
./TensileLite.out -d 3 -m 8192 -n  480 -k 8192 -r 1 -s 1 -x 4 -y 1 -u 64 -v 0 -l 1 -f 2
