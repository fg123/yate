// Represents a pane in a window.
#ifndef PANE_H
#define PANE_H

#include <ncurses.h>
#include <string>

#include "logging.h"
#include "navigate-window-provider.h"
#include "focusable.h"

using uint = unsigned int;

struct Pane : public NavigateWindowProvider {
  uint x;
  uint y;
  uint width;
  uint height;
  WINDOW *internal_window;
  Pane *parent;

  virtual void draw() = 0;
  void resize(uint nx, uint ny, uint nwidth, uint nheight) {
    Logging::info << "Pane resize: " << nx << ", " << ny << ", " << nwidth
                  << ", " << nheight << std::endl;
    onResize(nx, ny, nwidth, nheight);
    x = nx;
    y = ny;
    width = nwidth;
    height = nheight;
    wresize(internal_window, height, width);
    mvwin(internal_window, y, x);
  }
  // onResize should be called before updated, so we can do comparison
  virtual void onResize(uint nx, uint ny, uint nwidth, uint nheight) {}
  virtual const std::string &getTitle() = 0;
  // Pane(Pane *parent, const YateConfig_State_Pane &fromConfig)
  //     : Pane(parent, fromConfig.x(), fromConfig.y(), fromConfig.width(),
  //            fromConfig.height()) {}
  Pane(Pane *parent, int x, int y, int width, int height)
      : x(x), y(y), width(width), height(height), parent(parent) {
    internal_window = newwin(height, width, y, x);
    keypad(internal_window, true);
  }

  void titleUpdated() {
    Logging::breadcrumb("Pane Title Updated");
    onTitleUpdated();
    if (parent) {
      parent->titleUpdated();
    } else {
      Logging::breadcrumb("No parent to notify!");
    }
  }

  void mouseEvent(MEVENT *event) {
    Logging::info << "Mouse Event (" << event->x << ", " << event->y << " ,"
                  << event->z << ")" << std::endl;
    onMouseEvent(event);
  }

  void focusRequested(Pane *pane) {
    Logging::breadcrumb("Pane Focus Requested");
    onFocusRequested(pane);
    if (parent) {
      parent->focusRequested(pane);
    } else {
      Logging::breadcrumb("No parent to notify!");
    }
  }

  virtual void onFocusRequested(Pane *pane) {}
  virtual Focusable *getCurrentFocus() = 0;

  virtual void onMouseEvent(MEVENT *event) {}
  virtual void onTitleUpdated() {}
  virtual std::ostream &serialize(std::ostream &stream) { return stream; }
  virtual ~Pane() { delwin(internal_window); }
};

#endif
