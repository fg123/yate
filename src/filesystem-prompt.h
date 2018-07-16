#ifndef FILESYSTEM_PROMPT_H
#define FILESYSTEM_PROMPT_H

#include <string>

#include "editor.h"
#include "filesystem.h"
#include "prompt-window.h"
#include "util.h"

class FileSystemWindow : public PromptWindow {
  std::string title;
  Editor *editor = nullptr;
  Directory directory;
  std::function<void(std::string path)> callback;

  void finish(std::string path) {
    callback(path);
    yate.exitPrompt();
  }

 public:
  FileSystemWindow(Yate &yate, Editor *editor, std::string path,
                   std::function<void(std::string path)> callback)
      : PromptWindow(yate),
        editor(editor),
        directory(path),
        callback(callback) {
    title = "Choose File (" + path + ")";
  }
  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    std::string displayValue = directory.getDisplayString(index);
    return fuzzy_match(buffer, displayValue);
  }

  const std::string getItemString(size_t index) override {
    return directory.getDisplayString(index);
  }

  void onExecute(size_t index) override {
    if (directory.isDirectory(index)) {
      yate.enterPrompt(
          new FileSystemWindow(yate, editor, directory.getPath(index),
                               [this](std::string path) { finish(path); }));
    } else {
      finish(directory.getPath(index));
    }
  }

  const size_t getListSize() { return directory.size(); }
};

#endif
