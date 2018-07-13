#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <functional>
#include <string>

#include "editor.h"
#include "filesystem-prompt.h"
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
    //   be better implemented. It has a strange? memory leak with a string
    //   alloc?
    items.emplace_back(
        "File: Open", std::function<void()>([editor, &yate]() {
          yate.enterPrompt(new FileSystemWindow(yate, editor, "."));
        }));
    items.emplace_back("Edit: Undo", std::function<void()>([editor]() {
                         editor->onKeyPress(ctrl('z'));
                       }));
    items.emplace_back("Edit: Redo", std::function<void()>([editor]() {
                         editor->onKeyPress(ctrl('y'));
                       }));
  }
  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    return fuzzy_match(buffer, items.at(index).first);
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
