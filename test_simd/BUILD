cc_binary(
    name = "simple_add",
    srcs = ["simple_add.c"],
    # copts = ["-msse4.1"],
    copts = ["-O2 -msse"],
)

cc_binary(
    name = "neon_test",
    srcs = ["neon_test.c"],
    # copts = ["-O2 -mfloat-abi=softfp -mfpu=neon"],
)
