#ifndef RESIZE_PROMPT_H
#define RESIZE_PROMPT_H

#include "prompt-window.h"
#include "util.h"

class ResizePromptWindow : public PromptWindow {
  const std::string title = "Resize Pane:";
  size_t total_size;
  std::vector<size_t> index_map;
  std::vector<size_t> limits_map;
  std::vector<std::string> instructions;

 public:
  ResizePromptWindow(Yate &yate, NavigateWindow* parent) : PromptWindow(yate) {
    instructions.push_back("Hello Cecil");
  }

  const std::string &getTitle() override { return title; }

  void onKeyPress(int key) override {
    switch (key) {
      case KEY_UP:
      case KEY_DOWN:
      case KEY_LEFT:
      case KEY_RIGHT:
        break;
      case 27:
      case KEY_ENTER:
      case 10:
      case 13:
        yate.exitPrompt();
    }
  }

  bool match(std::string prompt_buf, size_t index) {
    return true;
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
