package(default_visibility=["//visibility:public"])

cc_library(
    name = "apu",
    hdrs = [
        "apu.h",
        "apu_dmc.h",
        "apu_noise.h",
        "apu_pulse.h",
        "apu_triangle.h",
    ],
    srcs = [
        "apu.cc",
        "apu_dmc.cc",
        "apu_noise.cc",
        "apu_pulse.cc",
        "apu_triangle.cc",
    ],
    deps = [
        ":nes-interface",
        ":mem-interface",
    ],
)

cc_library(
    name = "cartridge",
    hdrs = ["cartridge.h"],
    srcs = ["cartridge.cc"],
    deps = [
        ":nes-interface",
    ],
)

cc_library(
    name = "controller",
    hdrs = ["controller.h"],
    srcs = ["controller.cc"],
    deps = [
        ":nes-interface",
    ],
)

cc_library(
    name = "fm2",
    srcs = ["fm2.cc"],
    hdrs = ["fm2.h"],
    deps = [
        ":controller",
        ":nes-interface",
    ],
)

cc_library(
    name = "nes-interface",
    hdrs = ["nes.h"],
    deps = [
        "//src:io",
    ],
)
cc_library(
    name = "mem-interface",
    hdrs = ["mem.h"],
    deps = [
        "//src:memory",
    ]
)

cc_library(
    name = "mapper",
    hdrs = ["mapper.h"],
    srcs = ["mapper.cc"],
    deps = [
        ":nes-interface",
    ],
)

cc_library(
    name = "mapper-lib",
    hdrs = [
        "mapper1.h"
    ],
    srcs = [
        "mapper1.cc",
        "mapper2.cc",
    ],
    deps = [
        ":mapper",
        ":cartridge",
    ],
    alwayslink = 1,
)

cc_library(
    name = "mem",
    srcs = ["mem.cc"],
    deps = [
        ":apu",
        ":cartridge",
        ":controller",
        ":mapper",
        ":mem-interface",
        ":nes-interface",
        ":ppu",
    ],
)

cc_library(
    name = "nes",
    defines = [
        "USE_NEWCPU=1",
    ],
    srcs = ["nes.cc"],
    deps = [
        ":apu",
        ":cartridge",
        ":controller",
        ":fm2",
        ":mapper",
        ":mem",
        ":nes-interface",
        ":ppu",
        "//src/sdlutil:gfx",
        "//src:cpu2",
        "//src:debugger",
        "//src:io",
    ],
)

cc_library(
    name = "ppu",
    srcs = ["ppu.cc"],
    hdrs = ["ppu.h"],
    deps = [
        ":fm2",
        ":mem-interface",
        ":nes-interface",
        "//src:io",
    ],
)

cc_binary(
    name = "t1",
    srcs = ["t1.cc"],
    linkopts = [
        "-lSDL2",
        "-lpthread",
    ],
    deps = [
        ":nes",
        ":mapper-lib",
    ],
)
