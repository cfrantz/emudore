package(default_visibility=["//visibility:public"])

cc_library(
    name = "cpu",
    hdrs = ["cpu.h"],
    srcs = ["cpu.cpp"],
    deps = [
        ":memory",
        ":util",
    ],
)

cc_library(
    name = "cpu2",
    hdrs = ["cpu2.h"],
    srcs = ["cpu2.cc"],
    deps = [
        ":memory",
        ":util",
        "//external:gflags",
    ],
)

cc_library(
    name = "debugger",
    copts = [
        "-DUSE_NEWCPU=1"
    ],
    hdrs = ["debugger.h"],
    srcs = ["debugger.cpp"],
    deps = [
        ":cpu2",
        ":memory",
        ":util",
    ],
)

cc_library(
    name = "io",
    hdrs = ["io.h"],
    srcs = ["io.cpp"],
    deps = [
#        ":cpu",
        ":gamecontrollerdb",
        ":util",
        "//external:gflags",
        "//external:imgui",
    ],
)

cc_library(
    name = "gamecontrollerdb",
    hdrs = ["gamecontrollerdb.h"],
    srcs = ["gamecontrollerdb.cc"],
)

cc_library(
    name = "memory",
    hdrs = ["memory.h"],
)

cc_library(
    name = "util",
    hdrs = ["util.h"],
)

cc_binary(
    name = "emudore",
    srcs = ["main.cpp"],
    linkopts = [
        "-lSDL2",
    ],
    deps = [
        ":cpu",
        ":memory",
        ":io",
        "//src/c64:c64system",
    ]
)


cc_binary(
    name = "test_cpu",
    defines = [
        "TESTCPU=1",
    ],
    srcs = ["test_cpu.cc"],
    deps = [
        ":cpu2",
        ":memory",
        "//external:gflags",
    ],
    linkopts = [
        "-lpthread",
    ],
)














