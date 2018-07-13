#ifndef FILESYSTEM_PROMPT_H
#define FILESYSTEM_PROMPT_H

#include <string>

#include "editor.h"
#include "filesystem.h"
#include "prompt-window.h"
#include "util.h"

class FileSystemWindow : public PromptWindow<std::string> {
  std::string title;
  Editor *editor = nullptr;
  Directory directory;

 public:
  FileSystemWindow(Yate &yate, Editor *editor, std::string path)
      : PromptWindow(yate), editor(editor), directory(path) {
    title = "Choose File (" + path + ")";
  }
  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    return fuzzy_match(buffer, directory.getListing().at(index));
  }

  const std::string getItemString(size_t index) override {
    return directory.getListing().at(index);
  }

  void onExecute(size_t index) override {
    // Editor *tmp = editor;
    // yate.exitPromptThenRun(std::function<void()>([tmp](){

    // }));
  }

  const std::vector<std::string> &getItems() { return directory.getListing(); }
};

#endif
