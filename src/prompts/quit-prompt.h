#ifndef QUIT_PROMPT_H
#define QUIT_PROMPT_H

#include "prompt-window.h"
#include "buffer.h"

#include <vector>

class QuitPromptWindow : public PromptWindow {
  const std::string title = "You have unsaved changes! Select a buffer to save it: ";
  std::vector<Buffer*> unsaved_buffers;
  Yate &yate;

public:
  QuitPromptWindow(Yate& yate) : PromptWindow(yate), yate(yate) {
    for (auto buffer : yate.opened_buffers) {
      if (buffer->hasUnsavedChanges()) {
        unsaved_buffers.push_back(buffer);
      }
    }
    if (unsaved_buffers.empty()) {
      yate.quit();
    }
  }

  const std::string& getTitle() override {
    return title;
  }

  bool match(std::string buffer, size_t index) override {
    std::string str = getItemString(index);
    return fuzzy_match(buffer, str);
  }

  const std::string getItemString(size_t index) override {
    if (index == 0) {
      return "Quit without saving";
    }
    else if (index == 1) {
      return "Save all, then quit";
    }
    return unsaved_buffers.at(index - 2)->getFileName();
  }

  const size_t getListSize() {
    return unsaved_buffers.size() + 2;
  }

  void onExecute(size_t index) {
    if (index == 0) {
      yate.quit();
    }
    else if (index == 1) {
      for (auto buffer : unsaved_buffers) {
        buffer->writeToFile();
      }
      yate.quit();
    }
    else {
      unsaved_buffers.at(index - 2)->writeToFile();
      unsaved_buffers.erase(unsaved_buffers.begin() + index - 2);
      if (unsaved_buffers.empty()) {
        yate.quit();
      }
    }
  }
};

#endif