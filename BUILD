cc_binary(
		name = "yate",
		srcs = glob([
				"src/*.cc",
				"src/*.h",
		]),
		copts = [
				"-std=c++1z",
				"-g",
				"-Wall",
		],
		linkopts = [
				"-lncurses",
				"-lstdc++fs",
				"-fsanitize=address",
				"-fno-omit-frame-pointer",
		],
		deps = [":config_cc_proto"],
)

cc_proto_library(
		name = "config_cc_proto",
		deps = [":config_proto"],
)

proto_library(
		name = "config_proto",
		srcs = ["src/config.proto"],
)
