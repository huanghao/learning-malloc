# 使用Alpine Linux作为基础镜像（更小更快）
FROM alpine:latest

# 安装必要的工具
RUN apk update && apk add --no-cache \
    gcc \
    musl-dev \
    make \
    gdb \
    valgrind

# 设置工作目录
WORKDIR /app

# 设置默认命令
CMD ["make", "run"]
