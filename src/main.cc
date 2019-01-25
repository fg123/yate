#include <iostream>
#include <ncurses.h>

#include "yate.h"

int usage() {
  std::cerr << "Usage: yate [-c|--config configFile] [-l|--log logFile]"
            << std::endl;
  return 1;
}

int main(int argc, char *argv[]) {
  std::string yate_config_path;
  std::string log_path;
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (i != argc - 1 && (arg == "-c" || arg == "--config")) {
      yate_config_path = argv[++i];
    } else if (i != argc - 1 && (arg == "-l" || arg == "--log")) {
      log_path = argv[++i];
    } else {
      return usage();
    }
  }
  try {
    if (log_path.empty()) {
      log_path = "yate.log";
    }
    YateConfig config(yate_config_path);
    Logging::init(log_path);
    Yate yate(config);
  } catch (cpptoml::parse_exception e) {
    Logging::error << "Error parsing config TOML!" << std::endl;
  }
  Logging::cleanup();
  /* EndWin here instead of at Yate destructor */
  endwin();
  return 0;
}
