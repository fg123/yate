#include "pane-set.h"
#include "editor.h"
#include "navigate-prompt.h"
#include "tab-set.h"

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
      cumulative_width += new_width;
      cumulative_height += new_height;
      Logging::info << "Resizing Child Pos: (" << new_x << ", " << new_y
                    << ") with Size: (" << new_width << ", " << new_height
                    << ")" << std::endl;
      pane->resize(new_x, new_y, new_width, new_height);
    }
  }
  if (!bottom_right_pane) {
    safe_exit(2, "No bottom right pane!");
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
