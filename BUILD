cc_binary(
	name = "yate",
	srcs = glob(["src/*.cc", "src/*.h"]),
	linkopts = ["-lncurses"],
	copts = ["-std=c++11 -g -Wall"],
	deps = [":config_cc_proto"]
)

cc_proto_library(
	name = "config_cc_proto",
	deps = [":config_proto"]
)

proto_library(
	name = "config_proto",
	srcs = ["src/config.proto"]
)