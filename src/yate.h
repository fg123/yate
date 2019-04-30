#ifndef YATE_H
#define YATE_H

#include <functional>
#include <string>
#include <vector>
#include <deque>

#include "config.h"
#include "focusable.h"
#include "logging.h"
#include "theme.h"

class PaneSet;
class Buffer;
class PromptWindow;
class EditorNavigateProvider;

class Yate {
  std::vector<PromptWindow *> prompt_stack;
  Focusable *getCurrentFocus();
  bool should_quit = false;
  bool should_save_to_state = false;

  EditorNavigateProvider *lastEditorNavigateProvider = nullptr;

  void refreshAndStartCapture();

 public:
  YateConfig config;
  PaneSet *root;
  std::vector<Buffer *> opened_buffers;
  std::deque<std::string> clipboard_buffers;

  Yate(YateConfig config, std::istream &saved_state);
  Yate(YateConfig config, bool should_save_to_state,
       std::vector<std::string> &paths_to_open);
  ~Yate();

  void serialize(std::ostream &output);
  void onCapture(int result);
  Buffer *getBuffer(std::string path);
  bool isCurrentFocus(Focusable *focus) { return getCurrentFocus() == focus; }
  void enterPrompt(PromptWindow *window) { prompt_stack.push_back(window); }

  EditorNavigateProvider *getEditorNavigateProvider();

  void quit();
  void exitPrompt();
  void exitPromptThenRun(std::function<void()> function);
};

#endif
