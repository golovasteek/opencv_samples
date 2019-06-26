load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

new_local_repository(
    name="opencv",
    path="/home/enick/opt/",
    build_file="opencv.BUILD"
)

new_local_repository(
    name="cuda",
    path="/usr/",
    build_file="cuda.BUILD"
)

new_local_repository(
    name="egl",
    path="/usr/",
    build_file="egl.BUILD"
)

git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.8.1"
)
