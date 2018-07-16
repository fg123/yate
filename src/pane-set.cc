#include "pane-set.h"
#include "editor.h"
#include "tab-set.h"

#include <cmath>

PaneSet::PaneSet(Yate &yate, Pane *parent,
                 const YateConfig_State_PaneSet &fromConfig)
    : Pane(parent, fromConfig.pane()), yate(yate) {
  Logging::breadcrumb("Deserializing PaneSet");
  for (auto tab : fromConfig.tabsets()) {
    addPane(new TabSet(yate, this, tab));
  }
  for (auto editor : fromConfig.editors()) {
    addPane(new Editor(yate, this, editor));
  }
  for (auto paneset : fromConfig.panesets()) {
    addPane(new PaneSet(yate, this, paneset));
  }
}

PaneSet::~PaneSet() {
  for (auto pane : panes) {
    delete pane;
  }
}

void PaneSet::addPane(Pane *pane) {
  if (panes.size() == 0) {
    focused_pane = pane;
  }
  panes.emplace_back(pane);
}

void PaneSet::onResize(uint nx, uint ny, uint nwidth, uint nheight) {
  Logging::info << "PaneSet Resize (" << nx << ", " << ny << ", " << nwidth
                << ", " << nheight << ")" << std::endl;

  auto ratio_x = (double)nwidth / (double)(width);
  auto ratio_y = (double)nheight / (double)(height);
  for (auto pane : panes) {
    uint new_width = std::round(pane->width * ratio_x);
    uint new_height = std::round(pane->height * ratio_y);
    uint new_x = std::round((pane->x - x) * ratio_x) + nx;
    uint new_y = std::round((pane->y - y) * ratio_y) + ny;
    Logging::info << "Resizing Child Pos: (" << new_x << ", " << new_y
                  << ") with Size: (" << new_width << ", " << new_height << ")"
                  << std::endl;
    pane->resize(new_x, new_y, new_width, new_height);
  }
}
