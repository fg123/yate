#ifndef GOTO_LINE_PROMPT_H
#define GOTO_LINE_PROMPT_H
#include "prompt-window.h"

#include "util.h"

class GoToLinePromptWindow : public PromptWindow {
  Editor* editor;
  const std::string title = "Go to line:";

 public:
  GoToLinePromptWindow(Yate& yate, Editor* editor)
      : PromptWindow(yate), editor(editor) {}
  const std::string& getTitle() override { return title; }
  bool match(std::string buffer, size_t index) {
    std::string item = getItemString(index);
    return fuzzy_match(buffer, item);
  }

  const std::string getItemString(size_t index) {
    if (prompt_buffer.empty()) return "";
    return std::to_string(index + 1);
  }

  const size_t getListSize() {
    if (prompt_buffer.empty()) return 0;
    return editor->getBuffer()->size();
  }

  bool onEmptyExecute() override {
    std::string command = prompt_buffer;
    for (size_t i = 0, j = 0; i < command.size(); i++) {
      if (!std::isspace(command[i])) {
        command[j++] = command[i];
      }
    }
    if (command.size() >= 2) {
      char first = command[0];
      int distance = std::stoi(command.substr(1));
      if (first == '+' || first == '-')
        editor->goToLineOffset((first == '+' ? 1 : -1) * distance);
    }
    return false;
  }

  void onExecute(size_t index) {
    editor->goToLine(index);
    yate.exitPrompt();
  }
};
#endif
