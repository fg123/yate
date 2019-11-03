#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <functional>
#include <string>

#include "editor.h"
#include "prompt-window.h"
#include "util.h"

class CommandPromptWindow : public PromptWindow {
  const std::string title = "Enter Command:";
  std::vector<std::string> items;
  Editor *editor = nullptr;
  std::vector<std::function<void()>> special_commands;
 public:
  CommandPromptWindow(Yate &yate, Editor *editor);

  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    return fuzzy_match(buffer, items.at(index));
  }

  const std::string getItemString(size_t index) override {
    return items.at(index);
  }

  void onExecute(size_t index) override {
    index += 1; // for how we remove the first one
    if (index >= ActionManager::get().actions.size()) {
      yate.exitPromptThenRun(special_commands[index - ActionManager::get().actions.size()]);
      return;
    }
    Yate& s_yate = yate;
    Editor* s_editor = editor;
    yate.exitPromptThenRun([&s_yate, s_editor, index] () mutable {
      ActionManager::get().actions[index].fn(s_yate, s_editor);
    });
  }

  const size_t getListSize() { return items.size(); }
};

#endif
