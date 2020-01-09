#ifndef LOG_PANE_H
#define LOG_PANE_H

#include <fstream>
#include <string>
#include <vector>

#include "logging.h"
#include "pane.h"
#include "yate.h"
#include "editor.h"

class LogPane : public Editor, public LogListener {
  const std::string title = "Yate Log";
  Yate &yate;

  void init();

 public:
  LogPane(Yate &yate, Pane *parent, int x, int y, int width, int height)
      : Editor(yate, parent, new Buffer(yate), x, y, width, height), yate(yate) {
    init();
  }

  LogPane(Yate &yate, Pane *parent, std::istream &saved_state)
      : Editor(yate, parent, new Buffer(yate), saved_state),
        yate(yate) {
    Logging::breadcrumb("Deserializing Log Pane");
    init();
  }

  ~LogPane();

  void draw() override;
  const std::string &getTitle() override { return title; }
  Focusable *getCurrentFocus() override { return this; }

  void onMouseEvent(MEVENT *event) override { }

  int capture() override;
	void onKeyPress(int key) override;

  void serialize(std::ostream &stream) override {
    stream << "logpane " << x << " " << y << " " << width << " " << height << " ";
    stream << std::endl;
  }

  void onMessage(std::string message) override {
    getBuffer()->append_no_history(message);
    while (getBuffer()->size() > 300) {
      getBuffer()->delete_line_no_history(0);
    }
  }

  void onFlushed() override {
    if (isVisible()) {
      draw();
    }
  }

  virtual void switchBuffer(Buffer *newBuffer) {
    paneset_parent->replaceChildWith<Editor>(paneset_parent_child, newBuffer);
  }
};

#endif
