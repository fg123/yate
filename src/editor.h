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

#include "src/config.pb.h"

class Editor : public Pane, public Focusable {
  Yate &yate;
  Buffer *buffer;
  LineNumber current_line = 0;
  ColNumber current_col = 0;
  uint window_start = 0;

  // For when you move cursor past an empty line
  ColNumber phantom_col_pos = 0;
  void updateColWithPhantom();
  void limit_line_col() {
    if (current_line < 0) current_line = 0;
    if (current_line >= buffer->size()) {
      current_line = buffer->size() - 1;
    }
    if (current_col < 0) current_col = 0;
    if (current_col >= buffer->getLineLength(current_line)) {
      current_col = buffer->getLineLength(current_line) - 1;
    }
  }

  void init() {
    buffer->registerEditor(this);
    if (!yate.hasFocus()) {
      Logging::info << "Yate has no focused; setting focus." << std::endl;
      yate.setFocus(this);
    }
  }

  void switchBuffer(std::string newPath) {
    buffer->unregisterEditor(this);
    buffer = yate.getBuffer(newPath);
    buffer->registerEditor(this);
    titleUpdated();
    limit_line_col();
  }

 public:
  Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width,
         int height)
      : Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
    init();
  }

  void draw() override;
  const std::string &getTitle() override;
  int capture() override;
  void onKeyPress(int key) override;
  void onMouseEvent(MEVENT *event) override {
    if (event->bstate & BUTTON1_PRESSED) {
      if (yate.isCurrentFocus(this)) {
        current_line = (event->y - y) + window_start;
        current_col = (event->x - x) - (buffer->getLineNumberFieldWidth() + 2);
        limit_line_col();
      } else {
        yate.setFocus(this);
      }
    }
  }
  std::ostream &serialize(std::ostream &stream) override {
    stream << "editor {" << std::endl;
    stream << x << " " << y << " " << width << " " << height << std::endl;
    stream << buffer->getPath() << std::endl;
    stream << "}" << std::endl;
    return stream;
  }

  Editor(Yate &yate, Pane *parent, const YateConfig_State_Editor &fromConfig)
      : Pane(parent, fromConfig.pane()), yate(yate) {
    Logging::breadcrumb("Deserializing Editor");
    buffer = yate.getBuffer(fromConfig.buffer_path());
    init();
  }
};

#endif
