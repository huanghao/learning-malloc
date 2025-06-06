# C语言学习 - Docker环境

这个项目提供了一个完整的Docker环境来在Linux中编译和运行C程序，特别适合在macOS上开发但需要在Linux环境中测试的场景。

## 文件说明

- `ex1.c` - 示例C程序，演示不同内存段的变量
- `Dockerfile` - Docker镜像构建文件
- `docker-compose.yml` - Docker Compose配置文件
- `Makefile` - 编译和调试工具
- `run.sh` - 便捷的管理脚本

## 快速开始

### 1. 构建Docker镜像
\`\`\`bash
./run.sh build
\`\`\`

### 2. 运行程序
\`\`\`bash
./run.sh run
\`\`\`

### 3. 进入容器shell进行交互式开发
\`\`\`bash
./run.sh shell
\`\`\`

## 可用命令

- `./run.sh build` - 构建Docker镜像
- `./run.sh run` - 编译并运行程序
- `./run.sh shell` - 进入容器的bash shell
- `./run.sh debug` - 使用gdb调试程序
- `./run.sh valgrind` - 使用valgrind检查内存泄漏
- `./run.sh clean` - 清理Docker资源
- `./run.sh help` - 显示帮助信息

## 在容器内的操作

进入容器后，你可以使用以下命令：

\`\`\`bash
# 编译程序
make

# 运行程序
make run

# 使用gdb调试
make debug

# 使用valgrind检查内存
make valgrind

# 清理编译文件
make clean
\`\`\`

## 环境特性

- 基于Ubuntu 22.04
- 预装gcc、make、gdb、valgrind
- 支持交互式调试
- 代码热重载（通过volume挂载）

## 程序说明

`ex1.c`程序演示了C语言中不同类型变量在内存中的存储位置：

- **data段**: 已初始化的全局变量和静态变量
- **bss段**: 未初始化的全局变量
- **stack**: 局部变量

这对于理解内存布局和malloc学习非常有帮助。