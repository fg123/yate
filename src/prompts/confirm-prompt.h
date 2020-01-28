#ifndef CONFIRM_PROMPT_H
#define CONFIRM_PROMPT_H

#include "prompt-window.h"
#include "util.h"
#include "pane-set.h"

#include <ncurses.h>

class ConfirmPromptWindow : public PromptWindow {
  const std::string title;
  std::function<void()> yes;
  std::function<void()> no;

 public:
  ConfirmPromptWindow(Yate &yate, const std::string& title, std::function<void()> yes, std::function<void()> no)
    : PromptWindow(yate), title(title), yes(yes), no(no) {


  }

  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) {
    return fuzzy_match(buffer, getItemString(index));
  }

  const std::string getItemString(size_t index) {
    if (index == 0) return "Yes";
    else return "No";
  }

  const size_t getListSize() {
    return 2;
  }

  void onExecute(size_t index) {
    if (index == 0) yes();
    else no();
    yate.exitPrompt();
  }
};
#endif
