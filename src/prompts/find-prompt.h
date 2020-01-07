#ifndef FIND_PROMPT_H
#define FIND_PROMPT_H

#include "prompt-window.h"
#include "util.h"

class FindPromptWindow : public PromptWindow {
  const std::string title = "Find in file:";

  Editor *editor;
  Buffer *buffer;

 public:
  FindPromptWindow(Yate &yate, Editor *editor) : PromptWindow(yate), editor(editor), buffer(editor->getBuffer()) { }
  FindPromptWindow(Yate &yate, Editor *editor, const std::string& initial)
    : FindPromptWindow(yate, editor) { prompt_buffer = initial; }

  const std::string &getTitle() override { return title; }
  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, buffer->getLine(index));
  }
  const std::string getItemString(size_t index) {
    if (prompt_buffer.empty()) return "";
    return buffer->getLine(index);
  }
  const std::string getSyntaxHighlight(size_t index) {
    if (prompt_buffer.empty()) return "";
    return buffer->getSyntax(index);
  }
  const size_t getListSize() {
    if (prompt_buffer.empty()) return 0;
    return buffer->size();
  }
  void onExecute(size_t index) {
    ColNumber col;
    fuzzy_match(prompt_buffer, buffer->getLine(index), col);
    editor->goToLineCol(index, col);
    yate.exitPrompt();
  }
};
#endif
