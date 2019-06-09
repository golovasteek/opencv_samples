DEPS = [
    "//EGLStream:egl_streams",
    "@opencv//:opencv"
]

cc_binary(
    name = "egl_consumer",
    deps = DEPS,
    srcs = [
        "egl_consumer.cpp",
    ]
)

cc_binary(
    name = "egl_producer",
    deps = DEPS,
    srcs = [
        "egl_producer.cpp"
    ]
)