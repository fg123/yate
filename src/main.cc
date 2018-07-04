#include <iostream>

#include "yate.h"

int usage() {
	std::cerr << "Usage: yate [-c|--config configFile]" << std::endl;
	return 1;
}

int main(int argc, char *argv[]) {
	std::string pane_configuration_path;
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (i != argc - 1 && (arg == "-c" || arg == "--config")) {
			pane_configuration_path = argv[++i];
		}
		else {
			return usage();
		}
		
	}
	if (pane_configuration_path.empty()) {
		// User did not specify
		pane_configuration_path = ".yate";
	}
	Yate yate(pane_configuration_path);
	return 0;
}
