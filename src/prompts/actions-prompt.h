#ifndef ACTIONS_PROMPT_H
#define ACTIONS_PROMPT_H

#include <functional>

#include "prompt-window.h"
#include "util.h"
#include "actions.h"

class ActionsPromptWindow : public PromptWindow {
  const std::string title = "Manage Actions / Keyboard Shortcuts:";

  std::vector<Action>& actions;

 public:
  ActionsPromptWindow(Yate& yate)
      : PromptWindow(yate), actions(ActionManager::get().actions) {

  }

  const std::string& getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    std::string item = getItemString(index);
    return fuzzy_match(buffer, item);
  }

  const std::string getItemString(size_t index) override {
    return actions[index].displayString;
  }

  void onExecute(size_t index) override {
    // TODO: implement action changer
    yate.exitPrompt();
  }

  const size_t getListSize() { return actions.size(); }
};

#endif
