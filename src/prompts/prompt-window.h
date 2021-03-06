#ifndef PROMPT_WINDOW_H
#define PROMPT_WINDOW_H

#include <ncurses.h>
#include <string>
#include <limits>
#include <vector>

#include "focusable.h"
#include "pane.h"
#include "yate.h"

// Prompt window is a window that shows a list of options and a textbox with
// matching and "auto-complete"

class PromptWindow : public Pane, public Focusable {
 protected:
  std::vector<size_t> get_matching_items() {
    std::vector<size_t> result;
    for (unsigned int i = 0; i < getListSize(); i++) {
      if (match(prompt_buffer, i)) result.push_back(i);
    }
    return result;
  }

  int highlighted_index = 0;

  Yate& yate;
  bool input_enabled = true;
  std::string prompt_buffer;

 public:
  PromptWindow(Yate& yate, int x, int y, int width, int height)
      : Pane((Pane*)(yate.root), x, y, width, height), yate(yate) {}

  PromptWindow(Yate& yate)
      : PromptWindow(yate, COLS / 4, LINES / 4, COLS / 2, LINES / 2) {}

  ~PromptWindow() { curs_set(1); }

  virtual void onResize() {
    // PromptWindows have their own resize
    Pane::resize(COLS / 4, LINES / 4, COLS / 2, LINES / 2);
  }

  int capture() override {
    curs_set(0);
    draw();
    curs_set(input_enabled);
    int val = mvwgetch(internal_window, 1, 1 + prompt_buffer.size());
    curs_set(0);
    return val;
  }

  void onKeyPress(int key) override {
    std::vector<size_t> matched_items = get_matching_items();
    switch (key) {
      case KEY_BACKSPACE:
      case 127:
        if (!prompt_buffer.empty()) {
          prompt_buffer.pop_back();
        }
        break;
      case KEY_UP:
        if (highlighted_index > 0) {
          highlighted_index--;
        } else {
          highlighted_index = std::numeric_limits<int>::max();
        }
        break;
      case KEY_DOWN:
        if ((size_t) highlighted_index < matched_items.size() - 1) {
          highlighted_index++;
        } else {
          highlighted_index = 0;
        }
        break;
      case 27:  // Escape
        yate.exitPrompt();
        break;
      case KEY_ENTER:
      case 10:
      case 13:
        if (highlighted_index >= 0) {
          /* Calculate actual hovered item */
          onExecute(matched_items.at(highlighted_index));
        } else if (!this->onEmptyExecute()) {
          yate.exitPrompt();
        }
        break;
    }
    if (std::isprint(key) && input_enabled) {
      prompt_buffer.push_back(static_cast<char>(key));
    }
  }

  void limitHighlightedIndex(std::vector<size_t>& matched_items) {
    if (highlighted_index >= 0 &&
        (unsigned int)highlighted_index >= matched_items.size()) {
      highlighted_index = matched_items.size() - 1;
    }

    if (highlighted_index < 0 && !matched_items.empty()) {
      highlighted_index = 0;
    }
  }

  void draw() override {
    wmove(internal_window, 1, 1);
    wclrtoeol(internal_window);
    mvwprintw(internal_window, 1, 1, prompt_buffer.c_str());

    std::vector<size_t> matched_items = get_matching_items();
    limitHighlightedIndex(matched_items);

    auto sub_height = height - 4;
    int start = highlighted_index - sub_height / 2;
    while (matched_items.size() - start < sub_height) start--;
    if (start < 0) start = 0;
    int print_row = 3;
    unsigned int end = start + sub_height;
    if (end > matched_items.size()) end = matched_items.size();
    for (unsigned int i = 0; i < sub_height; i++) {
      wmove(internal_window, print_row + i, 1);
      wclrtoeol(internal_window);
    }
    for (unsigned int i = start; i < end; i++) {
      wmove(internal_window, print_row, 1);
      wclrtoeol(internal_window);
      if ((int)i == highlighted_index) {
        wattron(internal_window, A_REVERSE);
      }
      std::string str = getItemString(matched_items.at(i));
      std::string raw_syntax = getSyntaxHighlight(matched_items.at(i));

      std::string line =
        tab_replace(str, str, yate.config.getTabSize());

      std::string syntax =
          tab_replace(raw_syntax, str, yate.config.getTabSize(),
                      (char)SyntaxHighlighting::Component::WHITESPACE);

      if (line.length() < width - 2) {
        line.insert(line.end(), width - 2 - line.length(), ' ');
        syntax.insert(syntax.end(), width - 2 - syntax.length(), SyntaxHighlighting::Component::NO_HIGHLIGHT);
      }
      if (!syntax.empty() && (int)i != highlighted_index) {
        for (ColNumber j = 0; j < line.size(); j++) {
          auto syntax_color = yate.config.getTheme()->map(
            (SyntaxHighlighting::Component)syntax.at(j));
          if (yate.should_highlight) wattron(internal_window, syntax_color);
          mvwaddch(internal_window, print_row, 1 + j, line.at(j));
          if (yate.should_highlight) wattroff(internal_window, syntax_color);
        }
      }
      else {
        mvwinsstr(internal_window, print_row, 1, line.c_str());
      }
      wattroff(internal_window, A_REVERSE);
      print_row++;
    }

    wmove(internal_window, 2, 1);
    whline(internal_window, 0, width - 2);
    wborder(internal_window, 0, 0, 0, 0, 0, 0, 0, 0);

    wattron(internal_window, A_DIM);
    mvwprintw(internal_window, 0, 1, getTitle().c_str());
    wattroff(internal_window, A_DIM);
    wrefresh(internal_window);
  }

  virtual const std::string& getTitle() override = 0;
  virtual bool match(std::string buffer, size_t index) = 0;
  virtual const std::string getItemString(size_t index) = 0;
  virtual const std::string getSyntaxHighlight(size_t index) { return ""; }
  virtual const size_t getListSize() = 0;

  // Overridable handler to for if we do our own processing
  virtual bool onEmptyExecute() { return false; }
  virtual void onExecute(size_t index) = 0;
  virtual Focusable* getCurrentFocus() override { return this; }

  // TODO: I feel like prompt window shouldn't be a pane, because these
  // shouldn't live here
  size_t getNavigationItemsSize() override { return 0; }
  std::string getNavigationItem(size_t index) { return ""; }
  virtual bool onNavigationItemSelected(size_t index, NavigateWindow* parent) {
    return false;
  }
};
#endif
