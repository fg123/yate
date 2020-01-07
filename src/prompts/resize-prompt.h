#ifndef RESIZE_PROMPT_H
#define RESIZE_PROMPT_H

#include "prompt-window.h"
#include "util.h"
#include "pane-set.h"
#include "navigate-prompt.h"

#include <ncurses.h>

class ResizePromptWindow : public PromptWindow {
  static const int _width = 50;
  static const int _height = 9;

  const std::string title = "Resize Pane:";
  size_t total_size;
  std::vector<size_t> index_map;
  std::vector<size_t> limits_map;
  std::vector<std::string> instructions;
  PaneSet *parent;
  Pane *child;
  const Direction directions[4];

 public:
  ResizePromptWindow(Yate &yate, PaneSet* paneset_parent, Pane* child)
    : PromptWindow(yate, (COLS - _width) / 2, (LINES - _height) / 2, _width, _height),
      parent(paneset_parent), child(child), directions { Direction::LEFT, Direction::TOP, Direction::RIGHT, Direction::BOTTOM }
  {
    rebuildInstructions();
  }

  void onResize() override {
    // ResizePromptWindows have their own resize
    Pane::resize((COLS - _width) / 2, (LINES - _height) / 2, _width, _height);
  }

  std::string makeLong(std::string left, int value) {
    std::string right = "< " + std::to_string(value) + " >";
    left.resize(_width - 2 - right.size(), ' ');
    left.append(right);
    return left;
  }

  void rebuildInstructions() {
    instructions.clear();
    instructions.push_back(makeLong("Left Border", child->getBorder(Direction::LEFT)));
    instructions.push_back(makeLong("Top Border", child->getBorder(Direction::TOP)));
    instructions.push_back(makeLong("Right Border", child->getBorder(Direction::RIGHT)));
    instructions.push_back(makeLong("Bottom Border", child->getBorder(Direction::BOTTOM)));
  }

  const std::string &getTitle() override { return title; }

  void onKeyPress(int key) override {
    std::vector<size_t> matched_items = get_matching_items();
    size_t index = matched_items.at(highlighted_index);
    switch (key) {
      case KEY_LEFT:
      case KEY_RIGHT:
        parent->resizePane(child, directions[index], key == KEY_RIGHT ? 1 : -1);
        rebuildInstructions();
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
