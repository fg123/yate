#include <iostream>
#include <ncurses.h>

#include "yate.h"

int usage() {
  std::cerr << "Usage: yate [arguments] [file ..]" << std::endl << std::endl
            << "Arguments:" << std::endl
            << "  [-c|--config configFile]     specify a config file" << std::endl
            << "  [-l|--log logFile]           specify log file (default: yate.log)" << std::endl
            << "  [-h|--help]                  show this message" << std::endl;
  return 1;
}

int main(int argc, char *argv[]) {
  std::vector<std::string> paths_to_open;
  std::string yate_config_path;
  std::string log_path;
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (i != argc - 1 && (arg == "-c" || arg == "--config")) {
      yate_config_path = argv[++i];
    } else if (i != argc - 1 && (arg == "-l" || arg == "--log")) {
      log_path = argv[++i];
    } else if (arg == "-h" || arg == "--help") {
      return usage();
    } else {
      paths_to_open.push_back(arg);
    }
  }
  try {
    if (log_path.empty()) {
      log_path = "yate.log";
    }
    YateConfig config(yate_config_path);
    Logging::init(log_path);
    Logging::info << KEY_LEFT << " " << KEY_UP << " " << KEY_RIGHT << " " << KEY_DOWN << std::endl;
    Logging::info << KEY_SLEFT << " " << KEY_UP << " " << KEY_SRIGHT << " " << KEY_DOWN << std::endl;
    if (paths_to_open.empty()) {
      paths_to_open.push_back("Untitled");
    }
    Yate yate(config, paths_to_open);
  } catch (cpptoml::parse_exception e) {
    Logging::error << "Error parsing config TOML!" << std::endl;
  }
  Logging::cleanup();
  /* EndWin here instead of at Yate destructor */
  endwin();
  return 0;
}
