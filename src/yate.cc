#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <map>

#include "editor-navigate-provider.h"
#include "editor.h"
#include "logging.h"
#include "prompt-window.h"
#include "quit-prompt.h"
#include "tab-set.h"
#include "util.h"
#include "yate.h"

#define DEFAULT_INDENTATION_SIZE 8

void Yate::refreshAndStartCapture() {
  refresh();
  root->resize(0, 0, COLS, LINES);
  root->draw();

  while (true) {
    if (should_quit) break;
    int result = getCurrentFocus()->capture();
    if (result != ERR) {
      /* Some ncurses capture error */
      onCapture(result);
    }
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

void Yate::determineConfigFromBuffer(Buffer *buffer) {
  Logging::info << "Trying to determine indentation config from buffer: "
                << buffer->getPath() << std::endl;
  std::map<int, int> spacing_map;
  std::tuple<int, int> largest_pair(std::make_tuple(-1, -1));
  std::tuple<int, int> second_largest_pair(std::make_tuple(-1, -1));

  // 100 lines of source code is probably enough to determine
  //   indentation.
  size_t limit = std::min((size_t) 100, buffer->size());
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
    config.setIndentationStyle(YateConfig::IndentationStyle::TAB);
  } else {
    Logging::info
        << "Spaces detected as most frequent. Finding GCD of two largest."
        << std::endl;
    config.setIndentationStyle(YateConfig::IndentationStyle::SPACE);
    int first = std::get<0>(largest_pair);
    int second = std::get<0>(second_largest_pair);
    if (first > 0 && second > 0) {
      config.setTabSize(std::__gcd(first, second));
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
  Logging::info << "Yate Capture " << result << " " << KEY_MOUSE << " "
                << ctrl('q') << std::endl;
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
    enterPrompt(new QuitPromptWindow(*this));
  }
}

void Yate::registerEditor(Editor* editor) {
  editors.push_back(editor);
}

void Yate::unregisterEditor(Editor* editor) {
  editors.erase(
      std::remove(editors.begin(), editors.end(), editor),
      editors.end());
}

void Yate::quit() {
  Logging::breadcrumb("Quit");
  should_quit = true;
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
