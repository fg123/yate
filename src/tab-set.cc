#include "tab-set.h"
#include "editor.h"
#include "logging.h"
#include "pane.h"
#include "yate.h"

TabSet::TabSet(Yate &yate, Pane *parent, int x, int y, int width, int height)
    : Pane(parent, x, y, width, height), yate(yate) {
  PaneSet *pane_s = new PaneSet(yate, this, x, y + 1, width, height - 1);
  Editor *editor = new Editor(yate, pane_s, yate.getBuffer("Untitled"), x,
                              y + 1, width, height - 1);
  pane_s->addPane(editor);
  tabs.emplace_back(pane_s);
}

TabSet::~TabSet() {
  for (auto tab : tabs) {
    delete tab;
  }
}

void TabSet::drawTabs() {
  // Draw Tab Bar
  curs_set(0);
  wmove(internal_window, 0, 0);
  std::string all_tabs = "";
  std::vector<int> flags;
  int selected_start_position = 0;
  int selected_len = 0;
  decltype(tabs)::size_type i = 0;
  for (auto tab : tabs) {
    int flag = A_REVERSE;
    if (i == selected_tab) {
      flag = A_NORMAL;
      selected_start_position = all_tabs.length();
      selected_len = tab->getTitle().length() + 2;
    }
    // TODO(anyone): Add indicator if file is not saved?
    std::string str_to_add(' ' + tab->getTitle() + ' ');
    all_tabs += str_to_add;
    flags.insert(flags.end(), str_to_add.length(), flag);
    i += 1;
  }
  int start = 0;
  if (all_tabs.size() > width) {
    int string_mid = selected_start_position + (selected_len / 2);
    start = string_mid - (width / 2);
    if (start < 0) start = 0;
    if (start > selected_start_position) start = selected_start_position;
    while (start + width > all_tabs.length()) {
      start -= 1;
    }
  } else {
    while (all_tabs.length() < width) {
      all_tabs += " ";
    }
    while (flags.size() < width) {
      flags.push_back(A_REVERSE);
    }
  }
  for (unsigned int i = 0; i < width; i++) {
    mvwaddch(internal_window, 0, i, all_tabs[start + i] | flags[start + i]);
  }
  wrefresh(internal_window);
  curs_set(1);
}

void TabSet::draw() {
  Logging::breadcrumb("Tabset Draw");
  drawTabs();
  tabs[selected_tab]->draw();
}

const std::string &TabSet::getTitle() { return tabs[selected_tab]->getTitle(); }

void TabSet::onTitleUpdated() {
  Logging::breadcrumb("Tabset onTitleUpdated");
  drawTabs();
}

void TabSet::onResize(uint nx, uint ny, uint nwidth, uint nheight) {
  for (auto tab : tabs) {
    // Leave gap for tab bar.
    tab->resize(nx, ny + 1, nwidth, nheight - 1);
  }
}

void TabSet::onMouseEvent(MEVENT *event) {
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

std::string TabSet::getNavigationItem(size_t index) {
  if (index == tabs.size()) {
    return "New Tab";
  }
  return "Tab " + std::to_string(index);
}

bool TabSet::onNavigationItemSelected(size_t index, NavigateWindow *parent) {
  if (index == tabs.size()) {
    PaneSet *pane_s = new PaneSet(yate, this, x, y + 1, width, height - 1);
    Editor *editor = new Editor(yate, pane_s, yate.getBuffer("Untitled"), x,
                            y + 1, width, height - 1);
    pane_s->addPane(editor);
    addTab(pane_s);
    selected_tab = tabs.size() - 1;
    return true;
  }
  yate.enterPrompt(new NavigateWindow(yate, tabs.at(index), parent));
  return false;
}