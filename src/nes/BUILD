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
        "//src:pbmacro",
        "//proto:apu",
        "//external:imgui",
    ],
)

cc_library(
    name = "cartridge",
    hdrs = ["cartridge.h"],
    srcs = ["cartridge.cc"],
    deps = [
        ":nes-interface",
        "//proto:mappers",
        "//external:gflags",
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
    name = "debug_console",
    hdrs = ["debug_console.h"],
    srcs = ["debug_console.cc"],
    deps = [
        "//external:imgui",
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
        ":debug_console",
        "//proto:nes",
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
        "//proto:mappers",
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
        "mapper3.cc",
        "mapper4.cc",
    ],
    deps = [
        ":mapper",
        ":cartridge",
        ":ppu",
        "//src:pbmacro",
        "//proto:mappers",
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
        "//proto:nes",
        "//external:imgui",
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
        ":debug_console",
        "//src/sdlutil:gfx",
        "//src:cpu2",
        "//src:debugger",
        "//src:io",
        "//external:imgui",
    ],
)

cc_library(
    name = "ppu",
    srcs = ["ppu.cc"],
    hdrs = ["ppu.h"],
    deps = [
        ":cartridge",
        ":fm2",
        ":mapper",
        ":mem-interface",
        ":nes-interface",
        "//proto:ppu",
        "//src:pbmacro",
        "//src:io",
        "//external:imgui",
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
