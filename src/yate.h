#ifndef YATE_H
#define YATE_H

#include <deque>
#include <functional>
#include <string>
#include <vector>
#include <deque>

#include "config.h"
#include "focusable.h"
#include "logging.h"
#include "theme.h"
#include "actions.h"
#include "filesystem-indexer.h"

class PaneSet;
class Buffer;
class PromptWindow;
class EditorNavigateProvider;

class Yate {
  std::vector<PromptWindow *> prompt_stack;

  std::vector<PromptWindow *> delete_queue;

  Focusable *getCurrentFocus();
  bool should_quit = false;
  bool should_save_to_state = false;

  EditorNavigateProvider *lastEditorNavigateProvider = nullptr;

  void refreshAndStartCapture();
  void draw();

  std::deque<std::function<void()>> queuedCalls;

 public:
  // FilesystemIndexer filesystemIndexer;

  YateConfig config;
  PaneSet *root;
  bool should_highlight;

  std::vector<Buffer *> opened_buffers;
  std::vector<Editor *> editors;
  std::deque<std::string> clipboard_buffers;

  Yate(YateConfig config, bool should_highlight, std::istream &saved_state);
  Yate(YateConfig config, bool should_highlight, bool should_save_to_state,
       std::vector<std::string> &paths_to_open);
  ~Yate();

  void serialize(std::ostream &output);
  void onCapture(int result);
  Buffer *getBuffer(std::string path = "Untitled");
  bool isCurrentFocus(Focusable *focus) { return getCurrentFocus() == focus; }
  void enterPrompt(PromptWindow *window) { prompt_stack.push_back(window); }
  void determineConfigFromBuffer(Buffer *buffer);
  EditorNavigateProvider *getEditorNavigateProvider();

  void registerEditor(Editor *editor);
  void unregisterEditor(Editor *editor);

  void quit();
  void exitPrompt();
  void exitPromptThenRun(std::function<void()> function);

  void queueNextTick(std::function<void()> function);

  void moveEditorToFront(Editor *editor);
};

#endif
