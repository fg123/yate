#ifndef ACTIONS_PROMPT_H
#define ACTIONS_PROMPT_H

#include <functional>

#include "prompt-window.h"
#include "util.h"
#include "actions.h"

class ActionsPromptWindow : public PromptWindow {
  const std::string title = "Manage Actions / Keyboard Shortcuts:";

  std::vector<Action*>& actions;

  Action* editing_action = nullptr;

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
    if (actions[index] == editing_action)
      return actions[index]->pendingDisplayString;
    return actions[index]->displayString;
  }

  void onExecute(size_t index) override {
    editing_action = actions[index];
  }

  void onKeyPress(int key) override {
    if (editing_action) {
      switch (key) {
        case KEY_ESC:
          break;
        default:
          // TODO: check replacement
          ActionManager::get().changeActionKey(editing_action, key);
      }
      editing_action = nullptr;
      return;
    }
    PromptWindow::onKeyPress(key);
  }

  const size_t getListSize() { return actions.size(); }
};

#endif
