// A Pane Set is a set of panes arranged in some grid-like fashion.
// A Pane can be a Tab Set or an Editor.
#ifndef PANE_SET_H
#define PANE_SET_H

#include <vector>

#include "logging.h"
#include "pane.h"
#include "yate.h"

#include "src/config.pb.h"

class PaneSet : public Pane {
  Yate &yate;
  std::vector<Pane *> panes;
  // Store Focused Pane
  Pane *focused_pane;

 public:
  PaneSet(Yate &yate, Pane *parent, int x, int y, int width, int height)
      : Pane(parent, x, y, width, height), yate(yate) {}
  ~PaneSet();
  // TODO(anyone): Created better interface for proper splitting.
  void addPane(Pane *pane);
  void draw() {
    Logging::breadcrumb("Paneset Draw");
    for (auto pane : panes) {
      pane->draw();
    }
  }
  const std::vector<Pane *> &getPanes() { return panes; }
  const std::string &getTitle() { return focused_pane->getTitle(); }
  void onResize(uint nx, uint ny, uint nwidth, uint nheight) override;
  std::ostream &serialize(std::ostream &stream) override {
    stream << "paneset {" << std::endl;
    stream << x << " " << y << " " << width << " " << height << std::endl;
    for (auto pane : panes) {
      pane->serialize(stream);
    }
    stream << "}" << std::endl;
    return stream;
  }
  void onMouseEvent(MEVENT *event) override {
    for (auto pane : panes) {
      if (event->bstate & BUTTON1_PRESSED) {
        bool withinBounds = (uint)event->x >= pane->x &&
                            (uint)event->y >= pane->y &&
                            (uint)event->x < pane->x + pane->width &&
                            (uint)event->y < pane->y + pane->height;
        if (withinBounds) {
          focused_pane = pane;
          pane->mouseEvent(event);
        }
      }
    }
    titleUpdated();
  }
  PaneSet(Yate &yate, Pane *parent, const YateConfig_State_PaneSet &fromConfig);
};

#endif
