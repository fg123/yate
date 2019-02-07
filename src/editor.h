// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <fstream>
#include <string>
#include <vector>
#include "buffer.h"
#include "focusable.h"
#include "logging.h"
#include "pane-set.h"
#include "pane.h"
#include "yate.h"

const LineCol NO_SELECTION = std::make_tuple(-1, -1);

class Editor : public Pane, public Focusable {
  Yate &yate;
  Buffer *buffer;
  LineNumber current_line = 0;
  ColNumber current_col = 0;
  LineCol selection_start = NO_SELECTION;
  uint window_start_line = 0;
  uint window_start_col = 0;

  // For when you move cursor past an empty line
  ColNumber phantom_col_pos = 0;
  void updateColWithPhantom();
  void limitLine();
  void limitCol();
  void init();

  bool inSelection(LineNumber line, ColNumber col);
  void switchBuffer(std::string newPath);
  std::string generateStatusBar();

  /* Set to the closest pane_set parent */
  PaneSet *paneset_parent = nullptr;

  /* Set to the child of the aforementioned
   * pane_set that holds this instance of the editor */
  Pane *paneset_parent_child = nullptr;

 public:
  Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width,
         int height)
      : Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
    init();
  }

  Editor(Yate &yate, Pane *parent, std::istream &saved_state)
      : Pane(parent, saved_state),
        yate(yate),
        buffer(yate.getBuffer(read<std::string>(saved_state))) {
    Logging::breadcrumb("Deserializing Editor");
    init();
  }

  ~Editor() {
    buffer->unregisterEditor(this);
  }

  void switchBuffer(Buffer *newBuffer);
  void revertBuffer();

  void draw() override;
  const std::string &getTitle() override;
  Focusable *getCurrentFocus() override { return this; }
  Buffer *getBuffer() { return buffer; }
  void goToLine(LineNumber n);

  int capture() override;
  void onKeyPress(int key) override;
  void onMouseEvent(MEVENT *event) override;
  void serialize(std::ostream &stream) override {
    stream << "editor " << x << " " << y << " " << width << " " << height << " "
           << buffer->getPath() << " ";
    stream << std::endl;
  }
  size_t getNavigationItemsSize() override {
    /* Since the root of yate is a PaneSet, we can expect paneset_parent to
     * always be set. In this case there's always a child and we can always
     * split */
    return 3;
  }
  std::string getNavigationItem(size_t index) override {
    switch (index) {
      case 0:
        return "Focus";
      case 1:
        return "Split Vertically";
      case 2:
        return "Split Horizontally";
    }
    return "";
  }
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    switch (index) {
      case 0:
        focusRequested(this);
        break;
      case 1:
        paneset_parent->verticalSplit(paneset_parent_child);
        break;
      case 2:
        paneset_parent->horizontalSplit(paneset_parent_child);
        break;
    }
    return true;
  }
};

#endif
