cc_binary(
    name = "egl_consumer",
    deps = [
        "@cuda//:cuda",
        "@egl//:egl"
    ],
    srcs = [
        "egl_consumer.cpp",
        "egl_common.h",
        "egl_common.cpp"
    ]
)

cc_binary(
    name = "egl_producer",
    deps = [
        "@cuda//:cuda",
        "@egl//:egl"
    ],
    srcs = [
        "egl_producer.cpp",
        "egl_common.h",
        "egl_common.cpp"
    ]
)