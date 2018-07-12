#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <functional>
#include <string>

#include "editor.h"
#include "prompt-window.h"
#include "util.h"

using CommandPromptEntry = std::pair<std::string, std::function<void()>>;
class CommandPromptWindow : public PromptWindow<CommandPromptEntry> {
  const std::string title = "Enter Command:";
  std::vector<CommandPromptEntry> items;
  Editor *editor = nullptr;

 public:
  CommandPromptWindow(Yate &yate, Editor *editor)
      : PromptWindow(yate), editor(editor) {
    // TODO(felixguo): This seems super ghetto and should probably
    //   be better implemented.
    items.push_back(std::make_pair(
        "Edit: Undo", [editor]() { editor->onKeyPress(ctrl('z')); }));
    items.push_back(std::make_pair(
        "Edit: Redo", [editor]() { editor->onKeyPress(ctrl('y')); }));
  }
  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    return std::search(items.at(index).first.begin(),
                       items.at(index).first.end(), buffer.begin(),
                       buffer.end(), [](char ch1, char ch2) {
                         return std::toupper(ch1) == std::toupper(ch2);
                       }) != items.at(index).first.end();
  }

  const std::string getItemString(size_t index) override {
    return items.at(index).first;
  }

  void onExecute(size_t index) override {
    // TODO(felixguo): Implement proper focus stack instead of this.
    yate.exitPromptThenRun(items.at(index).second);
  }

  const std::vector<CommandPromptEntry> &getItems() { return items; }
};

#endif
