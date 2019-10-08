#ifndef RESIZE_PROMPT_H
#define RESIZE_PROMPT_H

#include "prompt-window.h"
#include "util.h"
#include "pane-set.h"

class ResizePromptWindow : public PromptWindow {
  const std::string title = "Resize Pane:";
  size_t total_size;
  std::vector<size_t> index_map;
  std::vector<size_t> limits_map;
  std::vector<std::string> instructions;
  PaneSet *parent;
  Pane *child;
  const Direction directions[4];

 public:
  ResizePromptWindow(Yate &yate, NavigateWindow* parent, PaneSet* paneset_parent, Pane* child) : PromptWindow(yate),
      parent(paneset_parent), child(child), directions{ Direction::LEFT, Direction::TOP, Direction::RIGHT, Direction::BOTTOM } {
    instructions.push_back("Done");
    instructions.push_back("Left Border");
    instructions.push_back("Top Border");
    instructions.push_back("Right Border");
    instructions.push_back("Bottom Border");
  }

  const std::string &getTitle() override { return title; }

  void onKeyPress(int key) override {
    std::vector<size_t> matched_items = get_matching_items();
    size_t index = matched_items.at(highlighted_index);
    switch (key) {
      case KEY_LEFT:
      case KEY_RIGHT:
        parent->movePane(child, directions[index - 1], key == KEY_LEFT ? 1 : -1);
        break;
    }
    PromptWindow::onKeyPress(key);
  }

  bool match(std::string buffer, size_t index) {
    return fuzzy_match(buffer, getItemString(index));
  }

  const std::string getItemString(size_t index) {
    return instructions[index];
  }

  const size_t getListSize() {
    return instructions.size();
  }

  void onExecute(size_t index) {
    yate.exitPrompt();
  }
};
#endif
