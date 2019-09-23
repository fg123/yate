#ifndef FIND_PROMPT_H
#define FIND_PROMPT_H

#include "prompt-window.h"
#include "util.h"
#include "syntax-lookup.h"

class SyntaxPromptWindow : public PromptWindow {
  const std::string title;

  Editor *editor;
  Buffer *buffer;

  std::vector<std::string> syntaxes;
 public:
  SyntaxPromptWindow(Yate &yate, Editor *editor) : PromptWindow(yate),
      title("Choose Syntax (" + buffer->getSyntax() + ")"), editor(editor), buffer(editor->getBuffer()) {
    for (auto syntax : SyntaxHighlighting::lookupMap) {
      syntaxes.push_back(syntax.first);
    }
  }

  const std::string &getTitle() override { return title; }

  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, syntaxes[index]);
  }

  const std::string getItemString(size_t index) {
    std::string str(syntaxes[index]);
    str[0] = std::toupper(str[0]);
    return str;
  }

  const size_t getListSize() {
    return syntaxes.size();
  }

  void onExecute(size_t index) {
    buffer->setSyntax(syntaxes[index]);
    yate.exitPrompt();
  }
};
#endif
