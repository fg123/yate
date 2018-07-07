#include "pane-set.h"
#include "editor.h"
#include "tab-set.h"

#include <cmath>

PaneSet::PaneSet(Yate &yate, Pane *parent,
                 const YateConfig_State_PaneSet &fromConfig)
    : Pane(parent, fromConfig.pane()), yate(yate) {
  Logging::breadcrumb("Deserializing PaneSet");
  for (auto tab : fromConfig.tabsets()) {
    addPane(new TabSet(yate, parent, tab));
  }
  for (auto editor : fromConfig.editors()) {
    addPane(new Editor(yate, parent, editor));
  }
  for (auto paneset : fromConfig.panesets()) {
    addPane(new PaneSet(yate, parent, paneset));
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
  Pane *bottom_right_pane = nullptr;
  uint cumulative_width = nx;
  uint cumulative_height = ny;
  auto apply = [nwidth, this, nx](uint a) {
    auto ratio = (double)width / (double)nwidth;
    return std::round((a - x) * ratio) + nx;
  };
  for (auto pane : panes) {
    Logging::info << "Paneset Resize: Bottom Right (" << pane->x + pane->width
                  << ", " << pane->y + pane->height
                  << ") with actual bottom at (" << x + width << ", "
                  << y + height << ")" << std::endl;
    if (pane->x + pane->width == x + width &&
        pane->y + pane->height == y + height) {
      // This should be the bottom right, we use this to account for rounding
      // errors.
      bottom_right_pane = pane;
    } else {
      uint new_width = apply(pane->width);
      uint new_height = apply(pane->height);
      cumulative_width += new_width;
      cumulative_height += new_height;
      pane->resize(apply(pane->x), apply(pane->y), new_width, new_height);
    }
  }
  if (!bottom_right_pane) {
    Logging::error << "Could not perform resize because we didn't find a "
                      "bottom right pane!"
                   << std::endl;
    return;
  }
  bottom_right_pane->resize(cumulative_width, cumulative_height,
                            nwidth - cumulative_width,
                            nheight - cumulative_height);
}
