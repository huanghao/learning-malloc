#!/bin/sh

# 编译目标文件
gcc -c -fPIC -o external.o external.c
gcc -c -fPIC -o main.o main.c

echo "---- 主文件里的重定位信息"
readelf -r main.o

# 编译共享库和可执行文件
gcc -shared -o libexternal.so external.o
gcc -o main main.o -L. -lexternal -Wl,-rpath=.

# 运行可执行文件
./main

echo "---- 可执行文件里的重定位信息"
readelf -r main

echo "---- 可执行文件里的符号表"
readelf -S main | grep -E '\.got|\.plt'
