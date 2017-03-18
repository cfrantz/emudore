package(default_visibility=["//visibility:public"])
load(":proto.bzl", "proto_library")

proto_library(
    name = "cpu6502",
    srcs = [
        "cpu6502.proto",
    ],
)

proto_library(
    name = "apu",
    srcs = [
        "apu.proto",
    ],
)

proto_library(
    name = "ppu",
    srcs = [
        "ppu.proto",
    ],
)

proto_library(
    name = "mappers",
    srcs = [
        "mappers.proto",
    ],
)

proto_library(
    name = "nes",
    srcs = [
        "nes.proto",
    ],
    deps = [
        ":cpu6502",
        ":apu",
        ":ppu",
        ":mappers",
    ],
)
