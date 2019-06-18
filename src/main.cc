#include <ncurses.h>
#include <iostream>

#include "yate.h"

int usage() {
  std::cerr << "Usage: yate [arguments] [file ..]" << std::endl
            << std::endl
            << "Arguments:" << std::endl
            << "  [-c|--config configFile]     specify a config file"
            << std::endl
            << "  [-l|--log logFile]           specify log file" << std::endl
            << "  [-s serializedFile]          specify a saved state" << std::endl
            << "  [--no-highlight]             disables syntax highlighting"<< std::endl
            << "  [-h|--help]                  show this message" << std::endl;
  return 1;
}

void init_curses() {
  set_escdelay(50);
  initscr();
  raw();
  noecho();
  nonl();
  start_color();
  keypad(stdscr, true);
  mousemask(BUTTON1_PRESSED, nullptr);
}

int main(int argc, char *argv[]) {
  std::vector<std::string> paths_to_open;
  std::string yate_config_path;
  std::string log_path;
  std::string saved_state_path;
  bool should_save_to_state = false;
  bool should_have_syntax_highlight = true;

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (i != argc - 1 && (arg == "-c" || arg == "--config")) {
      yate_config_path = argv[++i];
    } else if (i != argc - 1 && (arg == "-l" || arg == "--log")) {
      log_path = argv[++i];
    } else if (i != argc - 1 && arg == "-s") {
      saved_state_path = argv[++i];
    } else if (arg == "-h" || arg == "--help") {
      return usage();
    } else if (arg == "--init") {
      should_save_to_state = true;
    } else if (arg == "--no-highlight") {
      should_have_syntax_highlight = false;
    } else {
      paths_to_open.push_back(arg);
    }
  }
  init_curses();
  if (saved_state_path.empty()) {
    saved_state_path = ".yate";
  }
  if (yate_config_path.empty()) {
    yate_config_path = "yate.toml";
  }
  Logging::init(log_path);
  YateConfig config(yate_config_path);
  Logging::info << KEY_LEFT << " " << KEY_UP << " " << KEY_RIGHT << " "
                << KEY_DOWN << std::endl;
  Logging::info << KEY_SLEFT << " " << KEY_UP << " " << KEY_SRIGHT << " "
                << KEY_DOWN << std::endl;
  /* If paths given, we open paths; otherwise we check for saved state */
  std::ifstream saved_state(saved_state_path);
  try {
    if (!paths_to_open.empty()) {
      Yate yate(config, should_have_syntax_highlight, should_save_to_state,
                paths_to_open);
    } else if (saved_state.good()) {
      Yate yate(config, should_have_syntax_highlight, saved_state);
    } else {
      paths_to_open.push_back("Untitled");
      Yate yate(config, should_have_syntax_highlight, should_save_to_state,
                paths_to_open);
    }
  }
  catch (...) {
    Logging::cleanup();
    endwin();
    throw;
  }
  Logging::cleanup();
  endwin();
  return 0;
}
