#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <thread>
#include <chrono>

#include "editor-navigate-provider.h"
#include "editor.h"
#include "logging.h"
#include "prompt-window.h"
#include "quit-prompt.h"
#include "tab-set.h"
#include "util.h"
#include "yate.h"
#include "filesystem.h"

#define DEFAULT_INDENTATION_SIZE 8

void Yate::refreshAndStartCapture() {
  refresh();
  root->resize(0, 0, COLS, LINES - 1);
  draw();
  Logging::flush();

  while (true) {
    if (should_quit) break;
    while (!queuedCalls.empty()) {
      queuedCalls.front()();
      queuedCalls.pop_front();
    }
    int result = getCurrentFocus()->capture();
    if (result != ERR) {
      /* Some ncurses capture error */
      onCapture(result);
      Logging::flush();
    }
    std::this_thread::yield();
  }
}

Yate::Yate(YateConfig config, bool should_highlight, std::istream &saved_state)
    : config(config), should_highlight(should_highlight) {
  Logging::breadcrumb("=== Starting Yate ===");
  should_save_to_state = true;
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

Yate::Yate(YateConfig config, bool should_highlight, bool should_save_to_state,
           std::vector<std::string> &paths_to_open)
    : should_save_to_state(should_save_to_state),
      config(config),
      should_highlight(should_highlight) {
  Logging::breadcrumb("=== Starting Yate ===");
  root = new PaneSet(*this, nullptr, 0, 0, 1, 1);
  TabSet *tab_set = new TabSet(*this, root, 0, 0, 1, 1, paths_to_open);
  root->addPane(tab_set);
  // Given paths to open, check if there was config
  if (!config.wasLoadedFromFile) {
    // Determine from first opened buffer
    if (opened_buffers.size() > 0) {
      determineConfigFromBuffer(opened_buffers[0]);
    }
  }
  refreshAndStartCapture();
}

Yate::~Yate() {
  if (should_save_to_state) {
    std::ofstream config_file(".yate", std::fstream::trunc);
    serialize(config_file);
  }

  delete lastEditorNavigateProvider;

  delete root;
  for (auto buffer : opened_buffers) {
    delete buffer;
  }

  for (auto prompt : prompt_stack) {
    delete prompt;
  }
}

void Yate::draw() {
  root->draw();
  const static std::string bottom_bar = "yate v1.0 (Felix Guo) ";
  move(LINES - 1, 0);
  clrtoeol();
  mvaddstr(LINES - 1, std::max(0, (int)(COLS - bottom_bar.size())),
           bottom_bar.c_str());
  refresh();
}

void Yate::determineConfigFromBuffer(Buffer *buffer) {
  Logging::info << "Trying to determine indentation config from buffer: "
                << buffer->getPath() << std::endl;
  std::map<int, int> spacing_map;
  std::tuple<int, int> largest_pair(std::make_tuple(-1, -1));
  std::tuple<int, int> second_largest_pair(std::make_tuple(-1, -1));

  // 100 lines of source code is probably enough to determine
  //   indentation.
  size_t limit = std::min((size_t)100, buffer->size());
  for (LineNumber l = 0; l < limit; l++) {
    std::string &line = buffer->getLine(l);
    int key = 0;
    if (line.length() > 0 && line.at(0) == '\t') {
      key = -1;
    } else {
      ColNumber n = 0;
      while (n < line.length() && line.at(n) == ' ') {
        n++;
      }
      key = n;
    }
    if (key == 0) continue;
    spacing_map[key] += 1;
    if (key == std::get<0>(largest_pair)) {
      std::get<1>(largest_pair) = spacing_map[key];
    } else if (key == std::get<0>(second_largest_pair)) {
      std::get<1>(second_largest_pair) = spacing_map[key];
    } else if (spacing_map[key] > std::get<1>(largest_pair)) {
      second_largest_pair = largest_pair;
      largest_pair = std::make_tuple(key, spacing_map[key]);
    } else if (spacing_map[key] > std::get<1>(second_largest_pair)) {
      second_largest_pair = std::make_tuple(key, spacing_map[key]);
    }
  }
  Logging::info << "Two largest is (" << std::get<0>(largest_pair) << ", "
                << std::get<1>(largest_pair) << ") and ("
                << std::get<0>(second_largest_pair) << ", "
                << std::get<1>(second_largest_pair) << ")" << std::endl;
  if (std::get<0>(largest_pair) == -1) {
    Logging::info << "Tabs detected as most frequent starter." << std::endl;
    config.setIndentationStyle(IndentationStyle::TAB);
  } else {
    Logging::info
        << "Spaces detected as most frequent. Finding GCD of two largest."
        << std::endl;
    config.setIndentationStyle(IndentationStyle::SPACE);
    int first = std::get<0>(largest_pair);
    int second = std::get<0>(second_largest_pair);
    if (first > 0 && second > 0) {
      config.setTabSize(gcd(first, second));
    } else {
      config.setTabSize(first);
    }
  }
}

void Yate::moveEditorToFront(Editor *editor) {
  if (editor == editors[0]) return;
  for (size_t i = 1; i < editors.size(); i++) {
    if (editor == editors[i]) {
      editors.erase(editors.begin() + i);
      editors.insert(editors.begin(), editor);
      break;
    }
  }
}

Focusable *Yate::getCurrentFocus() {
  if (prompt_stack.empty())
    return root->getCurrentFocus();
  else
    return prompt_stack.back();
}

EditorNavigateProvider *Yate::getEditorNavigateProvider() {
  if (lastEditorNavigateProvider) {
    delete lastEditorNavigateProvider;
  }
  lastEditorNavigateProvider = new EditorNavigateProvider(*this);
  return lastEditorNavigateProvider;
}

Buffer *Yate::getBuffer(std::string path) {
  // Check if path is already opened?
  Logging::breadcrumb("GetBuffer: " + path);
  auto result =
      std::find_if(opened_buffers.begin(), opened_buffers.end(),
                   [path](Buffer *item) {
                     return path == item->getPath() ||
                      (isFileEquivalent(item->getPath(), path));
                  });
  if (result != opened_buffers.end()) {
    return *result;
  }

  Buffer *buffer = new Buffer(*this, path);
  opened_buffers.push_back(buffer);
  return buffer;
}

static MEVENT event;

void Yate::onCapture(int result) {
  Logging::info << "Yate Capture " << result << " " << KEY_MOUSE << " "
                << ctrl('q') << std::endl;
  if (result == KEY_RESIZE) {
    Logging::breadcrumb("KEY_RESIZE Hit!");
    refresh();
    root->resize(0, 0, COLS, LINES - 1);
    draw();
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
  for (unsigned int i = 0; i < delete_queue.size(); i++) {
    delete delete_queue[i];
  }
  delete_queue.clear();
}

void Yate::registerEditor(Editor *editor) { editors.push_back(editor); }

void Yate::unregisterEditor(Editor *editor) {
  editors.erase(std::remove(editors.begin(), editors.end(), editor),
                editors.end());
}

void Yate::quit() { should_quit = true; }

void Yate::exitPromptThenRun(std::function<void()> function) {
  exitPrompt();
  function();
}

void Yate::exitPrompt() {
  PromptWindow *p = prompt_stack.back();
  delete_queue.push_back(p);
  prompt_stack.pop_back();
  draw();
}

void Yate::queueNextTick(std::function<void()> function) {
  queuedCalls.push_back(function);
}
