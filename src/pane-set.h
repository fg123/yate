// A Pane Set is a set of panes arranged in some grid-like fashion.
// A Pane can be a Tab Set or an Editor.
#ifndef PANE_SET_H
#define PANE_SET_H

#include <vector>

#include "logging.h"
#include "pane.h"
#include "yate.h"

class PaneSet : public Pane {
  Yate &yate;
  std::vector<Pane *> panes;

 public:
  // Store Focused Pane
  Pane *focused_pane;
  PaneSet(Yate &yate, Pane *parent, std::istream &saved_state);
  PaneSet(Yate &yate, Pane *parent, int x, int y, int width, int height)
      : Pane(parent, x, y, width, height), yate(yate) {}
  ~PaneSet();
  // TODO(anyone): Created better interface for proper splitting.
  void addPane(Pane *pane);
  void draw();
  const std::vector<Pane *> &getPanes() { return panes; }
  const std::string &getTitle() {
    return focused_pane->getTitle();
  }
  Focusable *getCurrentFocus() override {
    return focused_pane->getCurrentFocus();
  }
  void onResize(uint nx, uint ny, uint nwidth, uint nheight) override;
  void serialize(std::ostream &stream) override;
  void onMouseEvent(MEVENT *event) override;
  void onFocusRequested(Pane *focus, Pane *child) override {
    focused_pane = child;
  }
  size_t getNavigationItemsSize() override;
  std::string getNavigationItem(size_t index) override;
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override;
};
#endif
