# Mach-O 格式学习 Demo

这个项目包含了一个用于学习和分析 Mach-O 文件格式的 demo 程序。

## 什么是 Mach-O

Mach-O (Mach Object) 是 macOS 和 iOS 系统上使用的可执行文件格式。它包含了：

- **Header**: 描述文件的基本信息（架构、文件类型等）
- **Load Commands**: 告诉系统如何加载和执行文件
- **Segments**: 内存段，包含代码、数据等
- **Sections**: 段内的具体区域

## 文件说明

- `mach_o_parser.c`: 主要的 Mach-O 解析器程序
- `simple_example.c`: 一个简单的示例程序，用于生成测试用的 Mach-O 文件
- `Makefile`: 编译脚本

## 构建和运行

### 编译解析器

```bash
make
```

### 运行测试

```bash
# 分析系统自带的二进制文件
./mach_o_parser /bin/ls

# 或者分析其他系统工具
./mach_o_parser /usr/bin/file
```

### 创建并分析自己的示例

```bash
# 编译示例程序
gcc -o simple_example simple_example.c

# 分析生成的 Mach-O 文件
./mach_o_parser simple_example
```

## 解析器功能

这个解析器可以分析：

1. **Mach-O Header 信息**
   - Magic number (32位/64位)
   - CPU 架构类型
   - 文件类型（可执行文件、动态库等）
   - Load commands 数量和大小

2. **Load Commands**
   - SEGMENT_64: 内存段定义
   - SYMTAB: 符号表
   - DYLD_INFO: 动态链接信息
   - MAIN: 程序入口点
   - 等等...

3. **Segments 和 Sections**
   - `__TEXT`: 代码段（可执行）
   - `__DATA`: 数据段（可读写）
   - `__LINKEDIT`: 链接信息
   - 各个 section 的详细信息

4. **Fat Binary 支持**
   - 通用二进制文件（包含多个架构）
   - 显示每个架构的信息

## 常见的 Mach-O 段和节

### __TEXT 段
- `__text`: 机器代码
- `__cstring`: C 字符串常量
- `__const`: 常量数据

### __DATA 段
- `__data`: 初始化的全局变量
- `__bss`: 未初始化的全局变量
- `__common`: 通用符号

### __LINKEDIT 段
- 包含链接时需要的信息（符号表、重定位信息等）

## 示例输出

运行 `./mach_o_parser /bin/ls` 的部分输出：

```
=== Mach-O Header ===
Magic: 0xfeedfacf (64-bit)
CPU Type: 16777228 (x86_64)
CPU Subtype: 3
File Type: 2 (Executable)
Number of load commands: 31
Size of load commands: 4648 bytes
Flags: 0x200085

=== Load Commands ===
Load Command #1:
  Command: 25 (SEGMENT_64)
  Segment name: __PAGEZERO
  VM address: 0x0
  VM size: 4294967296 bytes
  ...
```

## 扩展实验

你可以尝试：

1. 修改 `simple_example.c` 添加更多的全局变量和函数
2. 使用不同的编译选项（-O2, -g, -static）看看对 Mach-O 结构的影响
3. 分析动态库文件（.dylib）
4. 研究 Fat Binary 文件（如 `/usr/bin/lipo`）

## 相关工具

- `otool`: macOS 自带的目标文件分析工具
- `nm`: 显示符号表
- `file`: 显示文件类型
- `lipo`: 处理 Fat Binary 文件

## 参考资料

- [Apple Mach-O Programming Topics](https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/MachOTopics/)
- [Mach-O File Format Reference](https://github.com/aidansteele/osx-abi-macho-file-format-reference)
- `/usr/include/mach-o/loader.h`: Mach-O 结构定义

## 清理

```bash
make clean
```