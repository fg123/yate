#ifndef GOTO_LINE_PROMPT_H
#define GOTO_LINE_PROMPT_H
#include "prompt-window.h"

#include "util.h"

class GoToLinePromptWindow : public PromptWindow {
  Editor* editor;
  const std::string title = "Go to line:";

 public:
  GoToLinePromptWindow(Yate& yate, Editor *editor) : PromptWindow(yate), editor(editor) {

  }
  const std::string& getTitle() override { return title; }
  bool match(std::string buffer, size_t index) {
    std::string item = getItemString(index);
    return fuzzy_match(buffer, item);
  }

  const std::string getItemString(size_t index) {
    return std::to_string(index + 1);
  }

  const size_t getListSize() { return editor->getBuffer()->size(); }

  void onExecute(size_t index) {
    editor->goToLine(index);
    yate.exitPrompt();
  }
};
#endif
