#ifndef YATE_H
#define YATE_H

#include <functional>
#include <string>
#include <vector>

#include "config.h"
#include "focusable.h"
#include "logging.h"

class PaneSet;
class Buffer;
class PromptWindow;

class Yate {
  std::vector<PromptWindow *> prompt_stack;
  Focusable *getCurrentFocus();
  bool shouldQuit = false;

 public:
  YateConfig config;
  PaneSet *root;
  std::vector<Buffer *> opened_buffers;

  explicit Yate(YateConfig config);
  ~Yate();
  void onCapture(int result);
  Buffer *getBuffer(std::string path);
  bool isCurrentFocus(Focusable *focus) { return getCurrentFocus() == focus; }
  void enterPrompt(PromptWindow *window) { prompt_stack.push_back(window); }

  void quit();
  void exitPrompt();
  void exitPromptThenRun(std::function<void()> function);
};

#endif
