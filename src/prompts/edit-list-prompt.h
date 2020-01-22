#ifndef EDIT_LIST_PROMPT_H
#define EDIT_LIST_PROMPT_H

#include "prompt-window.h"
#include "util.h"

#include <algorithm>
#include <sstream>

template <class T>
class EditListPromptWindow : public PromptWindow {
  const std::string title =
    "Edit List (Press delete to remove item):";

  std::vector<T>& source;

 public:
  EditListPromptWindow(Yate &yate, std::vector<T>& source)
    : PromptWindow(yate), source(source) { }

  const std::string &getTitle() override { return title; }

  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, getItemString(index));
  }

  const std::string getItemString(size_t index) {
    if (!prompt_buffer.empty() && index == source.size()) {
      return "Add '" + prompt_buffer + "' to list";
    }
    return std::to_string(source.at(index));
  }

  const size_t getListSize() {
    if (prompt_buffer.empty())
      return source.size();
    return source.size() + 1;
  }

  void onKeyPress(int key) override {
    std::vector<size_t> matched_items = get_matching_items();
    size_t index = matched_items.at(highlighted_index);
    switch (key) {
      case KEY_DC:
        if (index != source.size()) {
          source.erase(source.begin() + index);
        }
      break;
    }
    PromptWindow::onKeyPress(key);
  }

  void onExecute(size_t index) {
    if (index == source.size()) {
      std::istringstream stream(prompt_buffer);
      source.push_back(read<T>(stream));
      std::sort(source.begin(), source.end());
    }
    else {
      yate.exitPrompt();
    }
  }
};
#endif
