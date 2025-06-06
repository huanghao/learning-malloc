#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  build     构建Docker镜像"
    echo "  run       运行程序"
    echo "  shell     进入容器shell"
    echo "  debug     使用gdb调试"
    echo "  valgrind  使用valgrind检查内存"
    echo "  clean     清理Docker资源"
    echo "  help      显示此帮助信息"
}

# 构建Docker镜像
build_image() {
    echo -e "${YELLOW}正在构建Docker镜像...${NC}"
    docker-compose build
}

# 运行程序
run_program() {
    echo -e "${GREEN}运行程序...${NC}"
    docker-compose run --rm c-learning make run
}

# 进入容器shell
enter_shell() {
    echo -e "${GREEN}进入容器shell...${NC}"
    docker-compose run --rm c-learning /bin/sh
}

# 使用gdb调试
debug_program() {
    echo -e "${YELLOW}启动gdb调试...${NC}"
    docker-compose run --rm c-learning make debug
}

# 使用valgrind检查内存
valgrind_check() {
    echo -e "${YELLOW}使用valgrind检查内存...${NC}"
    docker-compose run --rm c-learning make valgrind
}

# 清理Docker资源
clean_docker() {
    echo -e "${RED}清理Docker资源...${NC}"
    docker-compose down
    docker system prune -f
}

# 主逻辑
case "$1" in
    build)
        build_image
        ;;
    run)
        run_program
        ;;
    shell)
        enter_shell
        ;;
    debug)
        debug_program
        ;;
    valgrind)
        valgrind_check
        ;;
    clean)
        clean_docker
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo -e "${RED}未知选项: $1${NC}"
        show_help
        exit 1
        ;;
esac