#include "tab-set.h"
#include "editor.h"
#include "logging.h"
#include "pane.h"
#include "util.h"
#include "yate.h"

TabSet::TabSet(Yate &yate, Pane *parent, int x, int y, int width, int height,
               std::vector<std::string> &paths)
    : Pane(parent, x, y, width, height), yate(yate) {
  for (auto path : paths) {
    PaneSet *pane_s = new PaneSet(yate, this, x, y + 1, width, height - 1);
    Editor *editor = new Editor(yate, pane_s, yate.getBuffer(path), x, y + 1,
                                width, height - 1);
    pane_s->addPane(editor);
    tabs.emplace_back(pane_s);
  }
}

TabSet::TabSet(Yate &yate, Pane *parent, std::istream &saved_state)
    : Pane(parent, saved_state), yate(yate) {
  Logging::breadcrumb("Deserializing TabSet");
  int size = read<int>(saved_state);
  selected_tab = read<int>(saved_state);
  for (int i = 0; i < size; i++) {
    std::string type;
    saved_state >> type;
    if (type == "paneset") {
      addTab(new PaneSet(yate, this, saved_state));
    } else {
      std::cerr << "Unknown Tab Type: " << type << std::endl;
    }
  }
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

  tab_start.clear();
  tab_start.push_back(0);
  for (auto tab : tabs) {
    int flag = A_REVERSE;
    if (i == selected_tab) {
      flag = A_NORMAL;
      selected_start_position = all_tabs.length();
      selected_len = tab->getTitle().length() + 2;
    }
    std::string str_to_add(' ' + tab->getTitle() + ' ');
    all_tabs += str_to_add;
    tab_start.push_back(all_tabs.size());
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
  tab_draw_offset = start;
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

Focusable *TabSet::getCurrentFocus() {
  return tabs[selected_tab]->getCurrentFocus();
}

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
      uint click_pos = event->x - x + tab_draw_offset;
      Logging::breadcrumb("Tab Bar Clicked: resolved pos: " +
                          std::to_string(click_pos));
      int new_tab = -1;
      Logging::info << "0: " << tab_start[0] << std::endl;
      for (uint i = 1; i < tab_start.size(); i++) {
        Logging::info << i << ": " << tab_start[i] << std::endl;
        if (click_pos >= tab_start[i - 1] && click_pos < tab_start[i]) {
          new_tab = i - 1;
          break;
        }
      }
      if (new_tab == -1) {
        Logging::error << "Can't resolve tab select position!" << std::endl;
      } else {
        selected_tab = new_tab;
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

void TabSet::serialize(std::ostream &stream) {
  stream << "tabset " << x << " " << y << " " << width << " " << height << " "
         << tabs.size() << " " << selected_tab << std::endl;
  for (auto paneSet : tabs) {
    paneSet->serialize(stream);
  }
}

void TabSet::onFocusRequested(Pane *focus, Pane *child) {
  PaneSet *child_cast = dynamic_cast<PaneSet *>(child);
  if (!child_cast) {
    safe_exit(4, "Child of TabSet was not PaneSet!?");
  }
  selected_tab = indexOf(tabs, child_cast);
}
