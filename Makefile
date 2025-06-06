# 编译器
CC=gcc

# 编译选项
CFLAGS=-g -Wall -Wextra

# 目标文件名
TARGET=ex1

# 源文件
SOURCE=ex1.c

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# 运行程序
run: $(TARGET)
	./$(TARGET)

# 使用gdb调试
debug: $(TARGET)
	gdb ./$(TARGET)

# 使用valgrind检查内存
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# 清理编译文件
clean:
	rm -f $(TARGET)

.PHONY: all run debug valgrind clean
