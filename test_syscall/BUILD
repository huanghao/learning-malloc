cc_binary(
    name = "test_syscall",
    srcs = ["test_syscall.c"]
)

cc_binary(
    name = "test_threads_switch",
    srcs = ["test_threads_switch.c"],
    copts = ["-O2", "-pthread"],
    linkopts = ["-pthread"],
)

cc_binary(
    name = "test_spurious_wakeup",
    srcs = ["test_spurious_wakeup.c"],
    copts = ["-pthread"],
    linkopts = ["-pthread"]
)