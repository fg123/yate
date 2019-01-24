#include "pane-set.h"
#include "editor.h"
#include "navigate-prompt.h"
#include "tab-set.h"

#include <cmath>

// PaneSet::PaneSet(Yate &yate, Pane *parent,
//                  const YateConfig_State_PaneSet &fromConfig)
//     : Pane(parent, fromConfig.pane()), yate(yate) {
//   Logging::breadcrumb("Deserializing PaneSet");
//   for (auto tab : fromConfig.tabsets()) {
//     addPane(new TabSet(yate, this, tab));
//   }
//   for (auto editor : fromConfig.editors()) {
//     addPane(new Editor(yate, this, editor));
//   }
//   for (auto paneset : fromConfig.panesets()) {
//     addPane(new PaneSet(yate, this, paneset));
//   }
// }

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
  Pane *bottom_right_pane = nullptr;
  uint cumulative_width = 0;
  uint cumulative_height = 0;
  auto ratio_x = (double)nwidth / (double)(width);
  auto ratio_y = (double)nheight / (double)(height);
  for (auto pane : panes) {
    if (pane->x + pane->width == x + width &&
        pane->y + pane->height == y + height) {
      bottom_right_pane = pane;
    } else {
      uint new_width = std::round(pane->width * ratio_x);
      uint new_height = std::round(pane->height * ratio_y);
      uint new_x = std::round((pane->x - x) * ratio_x) + nx;
      uint new_y = std::round((pane->y - y) * ratio_y) + ny;
      cumulative_width += new_width;
      cumulative_height += new_height;
      Logging::info << "Resizing Child Pos: (" << new_x << ", " << new_y
                    << ") with Size: (" << new_width << ", " << new_height
                    << ")" << std::endl;
      pane->resize(new_x, new_y, new_width, new_height);
    }
  }
  if (!bottom_right_pane) {
    Logging::error << "No bottom right pane!" << std::endl;
    safe_exit(1);
  }
  cumulative_width %= nwidth;
  cumulative_height %= nheight;
  bottom_right_pane->resize(cumulative_width + nx, cumulative_height + ny,
                            nwidth - cumulative_width,
                            nheight - cumulative_height);
}

size_t PaneSet::getNavigationItemsSize() { return panes.size(); }
std::string PaneSet::getNavigationItem(size_t index) {
  Pane *pane = panes.at(index);
  // Shouldn't allow nesting of PaneSets?
  std::string name;
  if (dynamic_cast<TabSet *>(pane)) {
    name = "TabSet";
  } else if (dynamic_cast<Editor *>(pane)) {
    name = "Editor (" + dynamic_cast<Editor *>(pane)->getTitle() + ")";
  }
  return std::to_string(index) + ": " + name + " (" + std::to_string(pane->x) +
         ", " + std::to_string(pane->y) + ", " + std::to_string(pane->width) +
         ", " + std::to_string(pane->height) + ")";
}
bool PaneSet::onNavigationItemSelected(size_t index, NavigateWindow *parent) {
  yate.enterPrompt(new NavigateWindow(yate, panes.at(index), parent));
  return false;
}