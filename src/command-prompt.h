#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <functional>
#include <string>

#include "editor.h"
#include "buffer-select-prompt.h"
#include "filesystem-prompt.h"
#include "navigate-prompt.h"
#include "navigate-window-provider.h"
#include "pane-set.h"
#include "prompt-window.h"
#include "util.h"

using CommandPromptEntry = std::pair<std::string, std::function<void()>>;
class CommandPromptWindow : public PromptWindow {
  const std::string title = "Enter Command:";
  std::vector<CommandPromptEntry> items;
  Editor *editor = nullptr;

 public:
  CommandPromptWindow(Yate &yate, Editor *editor)
      : PromptWindow(yate), editor(editor) {
    items.emplace_back("File: Open", std::function<void()>([editor]() {
                         editor->onKeyPress(ctrl('o'));
                       }));
    items.emplace_back(
        "File: Open Buffer", std::function<void()>([&yate, editor]() {
          yate.enterPrompt(new BufferSelectPromptWindow(
              yate, std::function<void(int)>([&yate, editor](int bufferIndex) {
                editor->switchBuffer(yate.opened_buffers.at(bufferIndex));
              })));
        }));
    items.emplace_back("File: Save", std::function<void()>([editor]() {
                         editor->onKeyPress(ctrl('s'));
                       }));
    items.emplace_back("File: Quit",
                       std::function<void()>([&yate]() { yate.quit(); }));
    items.emplace_back("Edit: Undo", std::function<void()>([editor]() {
                         editor->onKeyPress(ctrl('z'));
                       }));
    items.emplace_back("Edit: Redo", std::function<void()>([editor]() {
                         editor->onKeyPress(ctrl('y'));
                       }));
    items.emplace_back(
        "Edit: Navigate", std::function<void()>([&yate]() {
          yate.enterPrompt(new NavigateWindow(yate, yate.root, nullptr));
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

  const size_t getListSize() { return items.size(); }
};

#endif
