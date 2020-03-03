#!/bin/bash
# ./phantom_test.sh > test_log.txt 2>&1


./TensileLite.out -d 1 -a 512 -b  512 -c  512 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 512 -b 1024 -c 1024 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 512 -b 2048 -c 2048 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 512 -b 4096 -c 4096 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 512 -b 8192 -c 8192 -m 1 -n 1 -x 1 -y 4 -u 32

./TensileLite.out -d 1 -a  512 -b 512 -c  512 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 1024 -b 512 -c 1024 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 2048 -b 512 -c 2048 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 4096 -b 512 -c 4096 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 8192 -b 512 -c 8192 -m 1 -n 1 -x 4 -y 1 -u 32

./TensileLite.out -d 1 -a 480 -b  512 -c  512 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 480 -b 1024 -c 1024 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 480 -b 2048 -c 2048 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 480 -b 4096 -c 4096 -m 1 -n 1 -x 1 -y 4 -u 32
./TensileLite.out -d 1 -a 480 -b 8192 -c 8192 -m 1 -n 1 -x 1 -y 4 -u 32

./TensileLite.out -d 1 -a  512 -b 480 -c  512 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 1024 -b 480 -c 1024 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 2048 -b 480 -c 2048 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 4096 -b 480 -c 4096 -m 1 -n 1 -x 4 -y 1 -u 32
./TensileLite.out -d 1 -a 8192 -b 480 -c 8192 -m 1 -n 1 -x 4 -y 1 -u 32
