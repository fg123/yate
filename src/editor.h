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

class Editor : public Pane, public Focusable {
  Yate &yate;
  Buffer *buffer;
  LineNumber current_line = 0;
  ColNumber current_col = 0;
  uint window_start = 0;

  // For when you move cursor past an empty line
  ColNumber phantom_col_pos = 0;
  void updateColWithPhantom();
  void limitLine();
  void limitCol();
  void init();

  void switchBuffer(std::string newPath);

 public:
  Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width,
         int height)
      : Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
    init();
  }

  void switchBuffer(Buffer* newBuffer);

  void draw() override;
  const std::string &getTitle() override;
  Focusable *getCurrentFocus() override { return this; }
  int capture() override;
  void onKeyPress(int key) override;
  void onMouseEvent(MEVENT *event) override;
  std::ostream &serialize(std::ostream &stream) override {
    stream << "editor {" << std::endl;
    stream << x << " " << y << " " << width << " " << height << std::endl;
    stream << buffer->getPath() << std::endl;
    stream << "}" << std::endl;
    return stream;
  }

  // Editor(Yate &yate, Pane *parent, const YateConfig_State_Editor &fromConfig)
  //     : Pane(parent, fromConfig.pane()), yate(yate) {
  //   Logging::breadcrumb("Deserializing Editor");
  //   buffer = yate.getBuffer(fromConfig.buffer_path());
  //   init();
  // }
  size_t getNavigationItemsSize() override { return 1; }
  std::string getNavigationItem(size_t index) override { return "Focus"; }
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    yate.setFocus(this);
    return true;
  }
};

#endif
