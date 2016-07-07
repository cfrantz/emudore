package(default_visibility = ["//visibility:public"])

cc_library(
    name = "imgui",
	linkopts = [
		"-lm",
		"-lGL",
    ],
    hdrs = [
        "imgui.h",
        "imconfig.h",
        "imgui_internal.h",
    ],
    srcs = [
        "imgui.cpp",
        "imgui_draw.cpp",
        "stb_rect_pack.h",
        "stb_textedit.h",
        "stb_truetype.h",
    ],
)
