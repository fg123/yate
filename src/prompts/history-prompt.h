#ifndef HISTORY_PROMPT_H
#define HISTORY_PROMPT_H

#include <functional>

#include "buffer.h"
#include "prompt-window.h"
#include "util.h"

class HistoryPromptWindow : public PromptWindow {
  const std::string title = "Select a history point:";
  Buffer *buffer;
  Editor *editor;

  EditNode *current_edit;

  std::vector<EditNode*> flattened;

  void traverseNode(EditNode* node) {
    while (node) {
      flattened.push_back(node);
      node = node->prev;
    }
  }
 public:
  HistoryPromptWindow(Yate& yate, Editor* editor)
      : PromptWindow(yate), editor(editor), current_edit(editor->getBuffer()->getCurrentEditNode()) {
    traverseNode(current_edit);
  }

  const std::string& getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    std::string item = getItemString(index);
    return fuzzy_match(buffer, item);
  }

  const std::string getItemString(size_t index) override {
    if (flattened[index] == current_edit) {
      return "> " + flattened[index]->getDescription();
    }
    return "  " + flattened[index]->getDescription();
  }

  void onExecute(size_t index) override {
    editor->fastTravel(flattened[index]);
    yate.exitPrompt();
  }

  const size_t getListSize() { return flattened.size(); }
};

#endif
