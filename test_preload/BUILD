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

# 演示脚本
sh_binary(
    name = "demo",
    srcs = ["demo.sh"],
    data = [
        ":main",
        ":memory_leak_demo",
        ":libpreload.so",
        ":libadvanced_preload.so",
    ],
)

# 测试目标 - 运行基本演示
sh_test(
    name = "basic_test",
    srcs = ["test_basic.sh"],
    data = [
        ":main",
        ":libpreload.so",
    ],
    size = "small",
)

# 测试目标 - 内存泄漏检测测试
sh_test(
    name = "memory_leak_test",
    srcs = ["test_memory_leak.sh"],
    data = [
        ":memory_leak_demo",
        ":libadvanced_preload.so",
    ],
    size = "small",
)