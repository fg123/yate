// Represents a pane in a window.
#ifndef PANE_H
#define PANE_H

#include <ncurses.h>
#include <iostream>
#include <string>

#include "focusable.h"
#include "logging.h"
#include "navigate-window-provider.h"
#include "util.h"

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
  Pane(Pane *parent, std::istream &source)
      : x(read<int>(source)),
        y(read<int>(source)),
        width(read<int>(source)),
        height(read<int>(source)),
        parent(parent) {
    Logging::breadcrumb("Deserializing Pane");
    init();
  }

  Pane(Pane *parent, int x, int y, int width, int height)
      : x(x), y(y), width(width), height(height), parent(parent) {
    init();
  }

  void init() {
    Logging::info << "Pane Init: " << x << " " << y << " " << width << " "
                  << height << std::endl;
    internal_window = newwin(height, width, y, x);
    if (!internal_window) {
      safe_exit(3, "Error allocating internal window!");
    }
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
    if (parent) {
      parent->focusRequested(pane);
      parent->onFocusRequested(pane, this);
    } else {
      Logging::breadcrumb("No parent to notify!");
    }
  }

  virtual void serialize(std::ostream &output) {
    output << x << " " << y << " " << width << " " << height << " ";
  }

  /* First argument is the pane that requested the focus, the second
   * is this current pane, so the parent can identify which child passed
   * the event up */
  virtual void onFocusRequested(Pane *paneRequestingFocus, Pane *child) {}
  virtual Focusable *getCurrentFocus() = 0;

  virtual void onMouseEvent(MEVENT *event) {}
  virtual void onTitleUpdated() {}
  virtual ~Pane() { delwin(internal_window); }

  template <typename T>
  T *findFirstParent() {
    std::vector<Pane *> useless;
    return findFirstParent<T>(useless);
  }

  template <typename T>
  T *findFirstParent(std::vector<Pane *> &parents) {
    static_assert(std::is_base_of<Pane, T>::value,
                  "findFirstParent<type> must be derived from Pane.");
    if (!parent) {
      return nullptr;
    }
    T *potential = dynamic_cast<T *>(parent);
    parents.push_back(this);
    if (potential) {
      return potential;
    } else {
      return parent->findFirstParent<T>(parents);
    }
  }

  void findAllParents(std::vector<Pane *> &parents) {
    parents.push_back(this);
    if (!parent) {
      return;
    }
    parent->findAllParents(parents);
  }
};

#endif
