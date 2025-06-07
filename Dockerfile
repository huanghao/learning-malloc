# 使用Alpine Linux作为基础镜像（更小更快）
FROM alpine:latest

# 启用edge分支以安装bazel
RUN echo "http://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories \
    && echo "http://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories

# 安装基础开发工具和bazel
RUN apk update && apk add --no-cache \
    gcc \
    g++ \
    musl-dev \
    make \
    gdb \
    python3 \
    python3-dev \
    py3-pip \
    curl \
    wget \
    unzip \
    git \
    bash \
    openjdk21 \
    bazel \
    && rm -rf /var/cache/apk/*

# 设置工作目录
WORKDIR /app

# 设置环境变量
ENV PATH="/usr/local/bin:/usr/bin:${PATH}"
ENV JAVA_HOME="/usr/lib/jvm/java-21-openjdk"

# 设置默认命令
CMD ["bazel", "build", "//..."]
