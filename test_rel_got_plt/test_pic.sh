# 编译成位置无关的目标文件（为了后续生成共享库）
echo "--- with PIC ----"
rm -f main.o
gcc -c -fPIC -o main.o main.c
readelf -r main.o

echo "--- without PIC ----"
rm -f main.o
gcc -c -o main.o main.c
readelf -r main.o
