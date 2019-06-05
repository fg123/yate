#include "pane-set.h"
#include "editor.h"
#include "navigate-prompt.h"
#include "tab-set.h"
#include "merge-prompt.h"

#include <cmath>

PaneSet::PaneSet(Yate &yate, Pane *parent, std::istream &saved_state)
    : Pane(parent, saved_state), yate(yate) {
  Logging::breadcrumb("Deserializing PaneSet");
  int size = read<int>(saved_state);
  int focusedIndex = read<int>(saved_state);
  for (int i = 0; i < size; i++) {
    std::string type;
    saved_state >> type;
    if (type == "tabset") {
      addPane(new TabSet(yate, this, saved_state));
    } else if (type == "editor") {
      addPane(new Editor(yate, this, saved_state));
    } else if (type == "paneset") {
      addPane(new PaneSet(yate, this, saved_state));
    } else {
      std::cerr << "Unknown Pane Type: " << type << std::endl;
    }
    if (i == focusedIndex) {
      focused_pane = panes.at(i);
    }
  }
}

PaneSet::~PaneSet() {
  for (auto pane : panes) {
    delete pane;
  }
}

void PaneSet::draw() {
  Logging::breadcrumb("Paneset Draw");
  for (auto pane : panes) {
    pane->draw();
  }
}

void PaneSet::doMerge(Pane *goner, Pane *stayer, SharedEdge edge,
                      size_t goner_index) {
  switch (edge) {
    case SharedEdge::LEFT:
      // Stayer is on the left of goner
      stayer->resize(stayer->x, stayer->y, stayer->width + goner->width,
        stayer->height);
      break;
    case SharedEdge::RIGHT:
      stayer->resize(goner->x, stayer->y,
        stayer->width + goner->width, stayer->height);
      break;
    case SharedEdge::TOP:
      stayer->resize(stayer->x, stayer->y, stayer->width,
        stayer->height + goner->height);
      break;
    case SharedEdge::BOTTOM:
      stayer->resize(stayer->x, goner->y, stayer->width,
        stayer->height + goner->height);
      break;
    case SharedEdge::NONE:
      Logging::error << "DoMerge called on invalid shared edge!" << std::endl;
      break;
  }
  panes.erase(panes.begin() + goner_index);
  delete goner;
  focused_pane = stayer;
}

void PaneSet::mergePane(Pane *child, NavigateWindow *navigateWindow) {
  // Show prompt for panes that share an edge with child.
  Logging::breadcrumb("Merge Pane");
  std::vector<std::pair<Pane*, SharedEdge>> bordering_siblings;
  size_t goner_index = 0;
  for (size_t i = 0; i < panes.size(); i++) {
    Pane* pane = panes[i];
    if (pane == child) goner_index = i;

    bool same_height = pane->height == child->height && pane->y == child->y;
    bool same_width = pane->width == child->width && pane->x == child->x;
    SharedEdge edge = SharedEdge::NONE;
    if (pane->x + pane->width == child->x && same_height) {
      edge = SharedEdge::LEFT;
    }
    else if (pane->x == child->x + child->width && same_height) {
      edge = SharedEdge::RIGHT;
    }
    else if (pane->y + pane->height == child->y && same_width) {
      edge = SharedEdge::TOP;
    }
    else if (pane->y == child->y + child->height && same_width) {
      edge = SharedEdge::BOTTOM;
    }
    if (edge != SharedEdge::NONE) {
      bordering_siblings.emplace_back(pane, edge);
    }
  }
  if (bordering_siblings.size() > 0) {
    // Prompt window to choose
    yate.enterPrompt(new MergePromptWindow(yate, this, child,
      bordering_siblings, goner_index, navigateWindow));
    return;
  }
  navigateWindow->finish();
}

void PaneSet::verticalSplit(Pane *child) {
  uint full = child->width;
  if (full <= 1) {
    return;
  }
  child->resize(child->x, child->y, full / 2, child->height);
  addPane(new Editor(yate, this, yate.getBuffer("Untitled"),
                     child->x + child->width, child->y, full - child->width,
                     child->height));
}

void PaneSet::horizontalSplit(Pane *child) {
  uint full = child->height;
  if (full <= 1) {
    return;
  }
  child->resize(child->x, child->y, child->width, full / 2);
  addPane(new Editor(yate, this, yate.getBuffer("Untitled"), child->x,
                     child->y + child->height, child->width,
                     full - child->height));
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
      Logging::info << "Resizing Child Pos: (" << new_x << ", " << new_y
                    << ") with Size: (" << new_width << ", " << new_height
                    << ")" << std::endl;
      if (pane->x + pane->width == x + width) {
        // Bordered on the right edge
        cumulative_height = std::max(cumulative_height, new_y + new_height - ny);
        Logging::info << "This child is bordered on the right edge, cumulative height now "
                      << cumulative_height << std::endl;
      }
      if (pane->y + pane->height == y + height) {
        // Bordered on the bottom edge
        cumulative_width = std::max(cumulative_width, new_x + new_width - nx);
        Logging::info << "This child is bordered on the bottom edge, cumulative width now "
                      << cumulative_width << std::endl;
      }
      pane->resize(new_x, new_y, new_width, new_height);
    }
  }
  if (!bottom_right_pane) {
    safe_exit(2, "No bottom right pane!");
  }
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

void PaneSet::onMouseEvent(MEVENT *event) {
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

void PaneSet::serialize(std::ostream &stream) {
  stream << "paneset " << x << " " << y << " " << width << " " << height << " "
         << panes.size() << " " << indexOf(panes, focused_pane) << std::endl;
  for (auto pane : panes) {
    pane->serialize(stream);
  }
}
