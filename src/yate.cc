#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#include "editor.h"
#include "logging.h"
#include "prompt-window.h"
#include "tab-set.h"
#include "util.h"
#include "yate.h"

#define DEFAULT_INDENTATION_SIZE 8

// TODO: this should probably get from the default.textproto file.
std::string default_config = R"(
state {
	root {
		pane {
			x: 0
			y: 0
			width: 2
			height: 2
		}
		tabsets {
			pane {
				x: 0
				y: 0
				width: 2
				height: 2
			}
			panesets {
				pane {
					x: 0
					y: 1
					width: 2
					height: 1
				}
				editors {
					pane {
						x: 0
						y: 1
						width: 2
						height: 1
					}
					buffer_path: "Untitled"
				}
			}
		}
	}
})";

Yate::Yate(YateConfig config) : config(config) {
  set_escdelay(50);
  initscr();
  raw();
  noecho();
  nonl();
  start_color();
  keypad(stdscr, true);
  mousemask(BUTTON1_PRESSED, nullptr);

  Logging::breadcrumb("=== Starting Yate ===");
  // int fd = open(config_path.c_str(), O_RDONLY);
  // if (fd < 0) {
  //   Logging::info << "No configuration provided. Defaulting." << std::endl;
  //   if (!google::protobuf::TextFormat::ParseFromString(default_config,
  //                                                      &config)) {
  //     Logging::error << "Error parsing default config text_proto!" << std::endl;
  //   }
  // } else {
  //   google::protobuf::io::FileInputStream stream(fd);
  //   stream.SetCloseOnDelete(true);
  //   google::protobuf::TextFormat::Parse(&stream, &config);
  // }
  // std::string output;
  // google::protobuf::TextFormat::PrintToString(config, &output);
  // Logging::info << output << std::endl;

  // TODO(felixguo): fix for serialized state storage
  // root = new PaneSet(*this, nullptr, config.state().root());
  root = new PaneSet(*this, nullptr, 0, 0, 1, 1);
  TabSet* tab_set = new TabSet(*this, root, 0, 0, 1, 1);
  root->addPane(tab_set);

  refresh();
  root->resize(0, 0, COLS, LINES);
  root->draw();

  if (!current_focus) {
    // TODO(felixguo): This might not be an actual issue.
    Logging::error << "No editor was initialized!" << std::endl;
    safe_exit(2);
  }

  while (true) {
    if (shouldQuit) break;
    onCapture(getCurrentFocus()->capture());
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

  for (auto prompt : prompt_stack) {
    delete prompt;
  }
}

Focusable *Yate::getCurrentFocus() {
  if (prompt_stack.empty())
    return root->getCurrentFocus();
  else
    return prompt_stack.back();
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

static MEVENT event;

void Yate::onCapture(int result) {
  Logging::info << result << " " << KEY_MOUSE << " " << ctrl('q') << std::endl;
  if (result == KEY_RESIZE) {
    Logging::breadcrumb("KEY_RESIZE Hit!");
    refresh();
    root->resize(0, 0, COLS, LINES);
    root->draw();
    for (auto prompt : prompt_stack) {
      // Prompts have their own "resize" function
      prompt->onResize();
    }
  } else if (result == KEY_MOUSE) {
    if (getmouse(&event) == OK) {
      root->onMouseEvent(&event);
    }
  } else {
    getCurrentFocus()->onKeyPress(result);
  }
  if (result == ctrl('q')) {
    shouldQuit = true;
  }
}

void Yate::quit() {
  Logging::breadcrumb("Quit");
  shouldQuit = true;
}

void Yate::exitPromptThenRun(std::function<void()> function) {
  exitPrompt();
  function();
}

void Yate::exitPrompt() {
  PromptWindow *p = prompt_stack.back();
  delete p;
  prompt_stack.pop_back();
  root->draw();
}
