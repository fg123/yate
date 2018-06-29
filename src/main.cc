#include <iostream>

#include "yate.h"

int usage() {
	std::cerr << "Usage: yate [-c|--config configFile]" << std::endl;
	return 1;
}

int main(int argc, char *argv[]) {
	Config config;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (i != argc - 1 && (arg == "-c" || arg == "--config")) {
			config.pane_configuration_path = argv[++i];
		}
		else {
			return usage();
		}
		
	}
	if (config.pane_configuration_path.empty()) {
		// User did not specify
		config.pane_configuration_path = ".yate";
	}
	Yate yate(config);
	return 0;
}
