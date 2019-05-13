cc_library(
    name="cuda",
    srcs=glob(["local/cuda-10.1/lib64/*.so*"]) + glob(["lib/x86_64-linux-gnu/libcuda.so*"]),
    hdrs=glob(
        ["local/cuda-10.1/include/**/*.hpp",
        "local/cuda-10.1/include/**/*.h"]
    ),
    strip_include_prefix="local/cuda-10.1/include",
    visibility = ["//visibility:public"]
)
