package(default_visibility=["//visibility:public"])

cc_library(
	name = "gfx",
	linkopts = [
		"-lSDL2_gfx",
	],
	hdrs = [ "gfx.h" ],
)


cc_library(
	name = "audiosample",
	linkopts = [
		"-lSDL2",
        "-lm",
        "-lpthread",
	],
	hdrs = [ "audiosample.h" ],
	srcs = [ "audiosample.cc" ],
)

cc_binary(
    name = "test_audio",
    srcs = ["test_audio.cc"],
    deps = [
        ":audiosample",
        "//src:io",
        "//external:gflags",
    ],
)
