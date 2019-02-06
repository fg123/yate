#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <functional>
#include <string>

#include "editor.h"
#include "prompt-window.h"
#include "util.h"

using CommandPromptEntry = std::pair<std::string, std::function<void()>>;
class CommandPromptWindow : public PromptWindow {
  const std::string title = "Enter Command:";
  std::vector<CommandPromptEntry> items;
  Editor *editor = nullptr;

 public:
  CommandPromptWindow(Yate &yate, Editor *editor);

  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    return fuzzy_match(buffer, items.at(index).first);
  }

  const std::string getItemString(size_t index) override {
    return items.at(index).first;
  }

  void onExecute(size_t index) override {
    yate.exitPromptThenRun(items.at(index).second);
  }

  const size_t getListSize() { return items.size(); }
};

#endif
