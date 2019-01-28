// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <fstream>
#include <string>
#include <vector>
#include "buffer.h"
#include "focusable.h"
#include "logging.h"
#include "pane.h"
#include "yate.h"

const LineCol NO_SELECTION = std::make_tuple(-1, -1);

class Editor : public Pane, public Focusable {
  Yate &yate;
  Buffer *buffer;
  LineNumber current_line = 0;
  ColNumber current_col = 0;
  LineCol selection_start = NO_SELECTION;
  uint window_start = 0;

  // For when you move cursor past an empty line
  ColNumber phantom_col_pos = 0;
  void updateColWithPhantom();
  void limitLine();
  void limitCol();
  void init();

  bool inSelection(LineNumber line, ColNumber col);
  void switchBuffer(std::string newPath);
  std::string generateStatusBar();

 public:
  Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width,
         int height)
      : Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
    init();
  }

  Editor(Yate &yate, Pane *parent, std::istream& saved_state)
      : Pane(parent, saved_state), yate(yate), buffer(yate.getBuffer(read<std::string>(saved_state))) {
    Logging::breadcrumb("Deserializing Editor");
    init();
  }

  void switchBuffer(Buffer* newBuffer);
  void revertBuffer();

  void draw() override;
  const std::string &getTitle() override;
  Focusable *getCurrentFocus() override { return this; }
  int capture() override;
  void onKeyPress(int key) override;
  void onMouseEvent(MEVENT *event) override;
  void serialize(std::ostream &stream) override {
    stream << "editor " << x << " " << y << " " << width << " " << height
           << " " << buffer->getPath() << " ";
  }
  size_t getNavigationItemsSize() override { return 1; }
  std::string getNavigationItem(size_t index) override { return "Focus"; }
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    focusRequested(this);
    return true;
  }
};

#endif
