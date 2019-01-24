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
  TabSet(Yate &yate, Pane *parent, int x, int y, int width, int height);
  ~TabSet();
  void addTab(PaneSet* paneset) {
    tabs.emplace_back(paneset);
  }
  void draw() override;
  const std::string &getTitle() override;
  void onTitleUpdated() override;
  void onResize(uint nx, uint ny, uint nwidth, uint nheight) override;
  std::ostream &serialize(std::ostream &stream) override {
    stream << "tabset {" << std::endl;
    stream << x << " " << y << " " << width << " " << height << std::endl;
    for (auto paneSet : tabs) {
      paneSet->serialize(stream);
    }
    stream << "}" << std::endl;
    return stream;
  }

  // TabSet(Yate &yate, Pane *parent, const YateConfig_State_TabSet &fromConfig)
  //     : Pane(parent, fromConfig.pane()), yate(yate) {
  //   Logging::breadcrumb("Deserializing TabSet");
  //   for (auto paneset : fromConfig.panesets()) {
  //     tabs.push_back(new PaneSet(yate, this, paneset));
  //   }
  // }
  void onMouseEvent(MEVENT *event) override {
    if (event->bstate & BUTTON1_PRESSED) {
      if ((uint)event->y == y && (uint)event->x >= x &&
          (uint)event->x < x + width) {
        Logging::breadcrumb("Tab Bar Clicked");
        // Tab Bar Click
        // TODO: Actually implement checking which tab pressed
        selected_tab += 1;
        if (selected_tab >= tabs.size()) {
          selected_tab = 0;
        }
        draw();
      } else {
        tabs[selected_tab]->mouseEvent(event);
      }
    }
  }
  size_t getNavigationItemsSize() override { return tabs.size() + 1; }
  std::string getNavigationItem(size_t index) override {
    if (index == tabs.size()) {
      return "New Tab";
    }
    return "Tab " + std::to_string(index);
  }
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    if (index == tabs.size()) {
      PaneSet *pane_s = new PaneSet(yate, this, x, y + 1, width, height - 1);
      Editor *editor = new Editor(yate, pane_s, yate.getBuffer("Untitled"), x,
                              y + 1, width, height - 1);
      return true;
    }
    yate.enterPrompt(new NavigateWindow(yate, tabs.at(index), parent));
    return false;
  }
};

#endif
