#ifndef YATE_H
#define YATE_H

#include <functional>
#include <string>
#include <vector>

#include "src/config.pb.h"

#include "focusable.h"
#include "logging.h"

class PaneSet;
class Buffer;
class PromptWindow;

class Yate {
  std::string config_path;
  YateConfig config;
  Focusable *current_focus = nullptr;
  std::vector<Buffer *> opened_buffers;
  std::vector<PromptWindow *> prompt_stack;
  Focusable *getCurrentFocus();

 public:
  PaneSet *root;

  explicit Yate(std::string config_path);
  ~Yate();
  bool onCapture(int result);
  Buffer *getBuffer(std::string path);
  void setFocus(Focusable *editor);
  bool hasFocus() { return current_focus; }
  void enterPrompt(PromptWindow *window) { prompt_stack.push_back(window); }

  int getTabSize();
  YateConfig_IndentationStyle getIndentationStyle();

  void exitPrompt();
  void exitPromptThenRun(std::function<void()> &function);
};

#endif
