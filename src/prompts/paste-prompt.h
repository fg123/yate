#ifndef PASTE_PROMPT_H
#define PASTE_PROMPT_H

#include "prompt-window.h"
#include "buffer.h"
#include "editor.h"

#include <vector>

class PasteWindow : public PromptWindow {
  const std::string title = "Paste from: ";
  Yate &yate;
  Editor* editor;

public:
  PasteWindow(Yate& yate, Editor* editor) : PromptWindow(yate), yate(yate), editor(editor) {
    
  }

  const std::string& getTitle() override {
    return title;
  }

  bool match(std::string buffer, size_t index) override {
    std::string str = getItemString(index);
    return fuzzy_match(buffer, str);
  }

  const std::string getItemString(size_t index) override {
    return std::to_string(index) + ": " + yate.clipboard_buffers.at(index);
  }

  const size_t getListSize() {
    return yate.clipboard_buffers.size();
  }

  void onExecute(size_t index) {
    editor->paste(yate.clipboard_buffers.at(index));
    yate.exitPrompt();
  }
};

#endif