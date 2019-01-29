// Represents a tabbed set.
#ifndef TAB_SET_H
#define TAB_SET_H

#include <ncurses.h>
#include <iostream>
#include <vector>

#include "navigate-prompt.h"
#include "pane-set.h"
#include "pane.h"
#include "util.h"

class TabSet : public Pane {
  Yate &yate;
  std::vector<PaneSet *> tabs;
  unsigned int selected_tab = 0;
  void drawTabs();

 public:
  TabSet(Yate &yate, Pane *parent, int x, int y, int width, int height, std::vector<std::string> &paths);
  TabSet(Yate &yate, Pane *parent, std::istream &saved_state);
  ~TabSet();
  void addTab(PaneSet* paneset) {
    tabs.emplace_back(paneset);
  }
  void draw() override;
  const std::string &getTitle() override;
  Focusable *getCurrentFocus() override;

  void onTitleUpdated() override;
  void onResize(uint nx, uint ny, uint nwidth, uint nheight) override;
  void serialize(std::ostream &stream) override;
  void onMouseEvent(MEVENT *event) override;
  void onFocusRequested(Pane *focus, Pane *child) override;
  size_t getNavigationItemsSize() override { return tabs.size() + 1; }
  std::string getNavigationItem(size_t index) override;
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override;
};

#endif
