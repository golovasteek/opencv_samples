cc_library(
    name="opencv",
    srcs=glob(["lib/*.so*"]),
    hdrs=glob(
        ["include/**/*.hpp",
        "include/**/*.h"]
    ),
    strip_include_prefix="include",
    visibility = ["//visibility:public"]
)
