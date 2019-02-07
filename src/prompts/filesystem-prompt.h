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
  FileSystemWindow *parent = nullptr;
  Directory directory;
  std::function<void(std::string path)> callback;

  void finish(std::string path) {
    auto cb = callback;
    yate.exitPromptThenRun([path, cb]() { cb(path); });
  }

 public:
  FileSystemWindow(Yate &yate, Editor *editor, std::string path, FileSystemWindow *parent,
                   std::function<void(std::string path)> callback)
      : PromptWindow(yate),
        editor(editor),
        parent(parent),
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
    if (index == 0) {
      if (parent) {
        yate.exitPrompt();
      }
      else {
        // We don't have a parent window, so we construct one */
        std::function<void(std::string path)> cb = callback;
        Yate &captured_yate = yate;
        Editor *captured_editor = editor;
        std::string path = directory.getPath(index);
        // We want to capture all these because *this* will be destroyed
        yate.exitPromptThenRun([&captured_yate, captured_editor, path, cb]() {
          captured_yate.enterPrompt(new FileSystemWindow(captured_yate,
            captured_editor, path, nullptr, cb));
        });
      }
    }
    else if (directory.isDirectory(index)) {
      yate.enterPrompt(
          new FileSystemWindow(yate, editor, directory.getPath(index),
          this, [this](std::string path) { finish(path); }));
    } else {
      finish(directory.getPath(index));
    }
  }

  const size_t getListSize() { return directory.size(); }
};

#endif
