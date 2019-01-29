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

void Yate::init() {
  set_escdelay(50);
  initscr();
  raw();
  noecho();
  nonl();
  start_color();
  keypad(stdscr, true);
  mousemask(BUTTON1_PRESSED, nullptr);

  Logging::breadcrumb("=== Starting Yate ===");
}

void Yate::refreshAndStartCapture() {
  refresh();
  root->resize(0, 0, COLS, LINES);
  root->draw();

  while (true) {
    if (shouldQuit) break;
    int result = getCurrentFocus()->capture();
    if (result != ERR) {
      /* Some ncurses capture error */
      onCapture(result);
    }
  }
}

Yate::Yate(YateConfig config, std::istream& saved_state) : config(config) {
  init();
  /* saved_state will start with paneset ... */
  std::string paneset;
  saved_state >> paneset;
  if (paneset != "paneset") {
    safe_exit(1, "Saved state was invalid; did not start with 'paneset'");
  }
  root = new PaneSet(*this, nullptr, saved_state);
  refreshAndStartCapture();
}

void Yate::serialize(std::ostream &output) {
  root->serialize(output);
  output << std::endl;
}

Yate::Yate(YateConfig config, std::vector<std::string>& paths_to_open) : config(config) {
  init();
  root = new PaneSet(*this, nullptr, 0, 0, 1, 1);
  TabSet* tab_set = new TabSet(*this, root, 0, 0, 1, 1, paths_to_open);
  root->addPane(tab_set);
  refreshAndStartCapture();
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

Buffer *Yate::getBuffer(std::string path) {
  // Check if path is already opened?
  Logging::breadcrumb("GetBuffer: " + path);
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
  Logging::info << "Yate Capture " << result << " " << KEY_MOUSE << " " << ctrl('q') << std::endl;
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
