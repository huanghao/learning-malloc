gcc -c foo.c -o foo.o
gcc -c main.c -o main.o

gcc -o main main.o foo.o

readelf -r main