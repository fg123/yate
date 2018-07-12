#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "editor.h"
#include "logging.h"
#include "prompt-window.h"
#include "tab-set.h"
#include "util.h"
#include "yate.h"

#include "src/config.pb.h"

#define DEFAULT_INDENTATION_SIZE 8

std::string default_config =
    R"(state {
	root {
		pane {
			x: 0
			y: 0
			width: 80
			height: 24
		}
		tabsets {
			pane {
				x: 0
				y: 0
				width: 80
				height: 24
			}
			panesets {
				pane {
					x: 0
					y: 1
					width: 80
					height: 23
				}
				editors {
					pane {
						x: 0
						y: 1
						width: 80
						height: 23
					}
					buffer_path: "Untitled"
				}
			}
		}
	}
})";

Yate::Yate(std::string config_path) : config_path(config_path) {
  set_escdelay(50);
  initscr();
  raw();
  noecho();
  nonl();
  start_color();
  keypad(stdscr, true);

  Logging::breadcrumb("=== Starting Yate ===");
  int fd = open(config_path.c_str(), O_RDONLY);
  if (fd < 0) {
    Logging::info << "No configuration provided. Defaulting." << std::endl;
    if (!google::protobuf::TextFormat::ParseFromString(default_config,
                                                       &config)) {
      Logging::error << "Error parsing default config text_proto!" << std::endl;
    }
  } else {
    google::protobuf::io::FileInputStream stream(fd);
    stream.SetCloseOnDelete(true);
    google::protobuf::TextFormat::Parse(&stream, &config);
  }
  std::string output;
  google::protobuf::TextFormat::PrintToString(config, &output);
  Logging::info << output << std::endl;
  root = new PaneSet(*this, nullptr, config.state().root());

  refresh();
  root->resize(0, 0, COLS, LINES);
  root->draw();

  if (!current_focus) {
    // TODO(felixguo): This might not be an actual issue.
    Logging::error << "No editor was initialized!" << std::endl;
    safe_exit(1);
  }
  while (true) {
    if (onCapture(current_focus->capture())) break;
  }
}

Yate::~Yate() {
  // TODO(felixguo): We might not want to truncate every time.
  // std::ofstream config_file(config_path, std::fstream::trunc);
  // root->serialize(config_file);

  delete root;
  for (auto buffer : opened_buffers) {
    delete buffer;
  }

  if (current_prompt) {
    // Current is the allocated prompt_window
    delete current_prompt;
  }
  endwin();
}

void Yate::setFocus(Focusable *focus) {
  Logging::info << "Setting current to: " << focus << std::endl;
  current_focus = focus;
}

Buffer *Yate::getBuffer(std::string path) {
  // Check if path is already opened?
  auto result =
      std::find_if(opened_buffers.begin(), opened_buffers.end(),
                   [path](Buffer *item) { return item->getPath() == path; });
  if (result != opened_buffers.end()) {
    return *result;
  }
  Buffer *buffer = new Buffer(*this, path);
  opened_buffers.push_back(buffer);
  return buffer;
}

bool Yate::onCapture(int result) {
  current_focus->onKeyPress(result);
  if (result == KEY_RESIZE) {
    Logging::breadcrumb("KEY_RESIZE Hit!");
    refresh();
    root->resize(0, 0, COLS, LINES);
    root->draw();
  }
  return result == ctrl('q');
}

void Yate::exitPromptThenRun(std::function<void()> &function) {
  exitPrompt();
  function();
}

int Yate::getTabSize() {
  return config.tab_size() > 0 ? config.tab_size() : DEFAULT_INDENTATION_SIZE;
}

YateConfig_IndentationStyle Yate::getIndentationStyle() {
  return config.indentation_style();
}

void Yate::exitPrompt() {
  if (previous_focus) {
    // TODO(anyone): Convert this prompt model to use unique pointers
    delete current_prompt;
    Logging::info << previous_focus << std::endl;
    Logging::info << current_focus << std::endl;
    setFocus(previous_focus);
    previous_focus = nullptr;
    current_prompt = nullptr;
  }
}
