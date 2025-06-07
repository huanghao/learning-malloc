# 使用Alpine Linux作为基础镜像（更小更快）
FROM alpine:latest

# 启用edge分支以安装bazel和最新软件包
RUN echo "http://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories \
    && echo "http://dl-cdn.alpinelinux.org/alpine/edge/testing" >> /etc/apk/repositories

# 安装基础开发工具、bazel、Go和Python
RUN apk update && apk add --no-cache \
    # C/C++ 开发工具
    gcc \
    g++ \
    musl-dev \
    make \
    gdb \
    # Go 开发环境
    go \
    # Python 开发环境
    python3 \
    python3-dev \
    py3-pip \
    py3-wheel \
    py3-setuptools \
    py3-numpy \
    py3-matplotlib \
    # 系统工具
    curl \
    wget \
    unzip \
    git \
    bash \
    vim \
    nano \
    tree \
    htop \
    sudo \
    # Java (for Bazel)
    openjdk21 \
    # Bazel 构建工具
    bazel \
    && rm -rf /var/cache/apk/*

# 创建Python虚拟环境
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

# 升级pip并安装常用Python包
RUN pip install --upgrade pip setuptools wheel && \
    pip install \
    pyelftools \
    rich \
    tqdm

# 设置Go环境变量
ENV GOROOT="/usr/lib/go"
ENV GOPATH="/app/go"
ENV PATH="${GOPATH}/bin:${GOROOT}/bin:${PATH}"

# 创建Go工作目录
RUN mkdir -p ${GOPATH}/{bin,src,pkg}

# 安装常用Go工具
RUN go install golang.org/x/tools/gopls@latest && \
    go install github.com/go-delve/delve/cmd/dlv@latest && \
    go install golang.org/x/tools/cmd/goimports@latest && \
    go install github.com/golangci/golangci-lint/cmd/golangci-lint@latest

# 设置工作目录
WORKDIR /app

# 设置环境变量
ENV PATH="/opt/venv/bin:/usr/local/bin:/usr/bin:${PATH}"
ENV JAVA_HOME="/usr/lib/jvm/java-21-openjdk"
ENV PYTHONPATH="/app:${PYTHONPATH}"

# 创建开发用户（可选，用于避免root权限问题）
RUN addgroup -g 1000 developer && \
    adduser -D -s /bin/bash -u 1000 -G developer developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && \
    mkdir -p /home/developer/.cache/bazel && \
    chown -R developer:developer /app ${GOPATH} /home/developer

# 设置代理环境变量（如果需要）
ENV HTTP_PROXY=""
ENV HTTPS_PROXY=""
ENV NO_PROXY="localhost,127.0.0.1"

# 复制配置文件（如果存在）
COPY --chown=developer:developer . /app/

# 切换到开发用户
USER developer

# 设置默认命令
CMD ["bash"]
