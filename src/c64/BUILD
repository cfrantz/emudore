package(default_visibility=["//visibility:public"])

cc_library(
    name = "c64system",
    hdrs = [
        "c64.h",
        "c64memory.h",
        "cia1.h",
        "cia2.h",
        "loader.h",
        "sid.h",
        "vic.h",
    ],
    srcs = [
        "c64.cpp",
        "c64memory.cpp",
        "cia1.cpp",
        "cia2.cpp",
        "loader.cpp",
        "vic.cpp",
    ],
    deps = [
        "//src:cpu",
        "//src:debugger",
        "//src:io",
        "//src:memory",
        "//src:util",
    ]
)
