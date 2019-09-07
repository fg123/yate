#ifndef FIND_ALL_PROMPT_H
#define FIND_ALL_PROMPT_H

#include "prompt-window.h"
#include "util.h"

class FindAllPromptWindow : public PromptWindow {
  const std::string title = "Find in all buffers:";
  size_t total_size;
  std::vector<size_t> index_map;
  std::vector<size_t> limits_map;
 public:
  FindAllPromptWindow(Yate &yate) : PromptWindow(yate), total_size(0) {
    limits_map.push_back(0);
    for (size_t i = 0; i < yate.opened_buffers.size(); i++) {
      auto buffer = yate.opened_buffers[i];
      total_size += buffer->size();
      limits_map.push_back(total_size);
      index_map.insert(index_map.end(), buffer->size(), i);
    }
  }

  const std::string &getTitle() override { return title; }
  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, getLine(index));
  }

  const std::string& getLine(size_t index) {
    size_t i = index_map[index];
    size_t line = index - limits_map[i];
    return yate.opened_buffers[index_map[index]]->getLine(line);
  }

  const std::string getItemString(size_t index) {
    size_t i = index_map[index];
    size_t line = index - limits_map[i];
    return yate.opened_buffers[index_map[index]]->getFileName() + ": " +
      yate.opened_buffers[index_map[index]]->getLine(line);
  }

  const size_t getListSize() {
    if (prompt_buffer.empty()) return 0;
    return total_size;
  }

  void onExecute(size_t index) {
    size_t i = index_map[index];
    size_t line = index - limits_map[i];
    Editor *editor = yate.opened_buffers[i]->getRegisteredEditors()[0];
    editor->focusRequested(editor);
    ColNumber col;
    fuzzy_match(prompt_buffer, getLine(index), col);
    editor->goToLineCol(line, col);
    yate.exitPrompt();
  }
};
#endif
