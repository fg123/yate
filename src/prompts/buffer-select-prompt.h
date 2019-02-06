#ifndef BUFFER_SELECT_PROMPT_H
#define BUFFER_SELECT_PROMPT_H

#include <functional>

#include "buffer.h"
#include "prompt-window.h"
#include "util.h"

class BufferSelectPromptWindow : public PromptWindow {
  const std::string title = "Select an open buffer:";
  std::vector<Buffer*>& buffers;
  std::function<void(int)> callback;

 public:
  BufferSelectPromptWindow(Yate& yate, std::function<void(int)> callback)
      : PromptWindow(yate), buffers(yate.opened_buffers), callback(callback) {
  }

  const std::string& getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    std::string displayValue = buffers.at(index)->getFileName();
    return fuzzy_match(buffer, displayValue);
  }

  const std::string getItemString(size_t index) override {
    return std::to_string(index + 1) + ": " + buffers.at(index)->getFileName();
  }

  void onExecute(size_t index) override {
    callback(index);
    yate.exitPrompt();
  }

  const size_t getListSize() { return buffers.size(); }
};

#endif
