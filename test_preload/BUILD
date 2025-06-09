# LD_PRELOAD Demo Build Configuration

# 主程序
cc_binary(
    name = "main",
    srcs = ["main.c"],
    deps = [],
)

# 内存泄漏演示程序
cc_binary(
    name = "memory_leak_demo",
    srcs = ["memory_leak_demo.c"],
    deps = [],
)

# 基本预加载库
cc_binary(
    name = "libpreload.so",
    srcs = ["preload.c"],
    linkshared = True,
    linkopts = ["-ldl"],
    copts = ["-fPIC"],
)

# 高级预加载库（内存泄漏检测）
cc_binary(
    name = "libadvanced_preload.so",
    srcs = ["advanced_preload.c"],
    linkshared = True,
    linkopts = ["-ldl"],
    copts = ["-fPIC"],
)
