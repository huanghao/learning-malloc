cc_binary(
    name = "ex1",
    srcs = ["ex1.c"],
    copts = [
        "-g",
        "-Wall",
        "-Wextra",
        "-D_GNU_SOURCE",  # 确保启用GNU扩展
    ],
)

cc_binary(
    name = "test_section",
    srcs = ["test_section.c"],
    copts = [
        "-g",
        "-Wall",
        "-Wextra",
    ],
)


cc_binary(
    name = "test_weak_sym",
    srcs = ["test_weak_sym.c", "weak.c"],
    copts = [
        "-g",
        "-Wall",
        "-Wextra",
    ],
)

cc_binary(
    name = "test_strong_sym",
    srcs = ["test_weak_sym.c", "strong.c"],
    copts = [
        "-g",
        "-Wall",
        "-Wextra",
    ],
)