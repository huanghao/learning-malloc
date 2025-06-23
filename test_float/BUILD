cc_library(
    name = "print_float_sem",
    srcs = [
        "print_float_sem.c",
        "print_float_sem.h",
    ],
)

cc_binary(
    name = "make_a_float",
    srcs = ["make_a_float.c"],
    deps = [":print_float_sem"],
)

cc_binary(
    name = "div_float0",
    srcs = ["div_float0.c"],
)

cc_binary(
    name = "div_int0",
    srcs = ["div_int0.c"],
)

cc_binary(
    name = "cmp_nan",
    srcs = ["cmp_nan.c"],
    linkopts = ["-lm"],
)

cc_binary(
    name = "cmp_float",
    srcs = ["cmp_float.c"],
)

cc_binary(
    name = "make_snan",
    srcs = ["make_snan.c"],
    deps = [":print_float_sem"],
)

cc_binary(
    name = "test_FP16",
    srcs = ["test_FP16.c"],
    copts = [
        "-O2",
        "-march=native",
    ],
)

cc_binary(
    name = "test_BF16",
    srcs = ["test_BF16.c"],
    copts = [
        "-O2",
        "-march=native",
    ],
)
