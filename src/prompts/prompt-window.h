#ifndef PROMPT_WINDOW_H
#define PROMPT_WINDOW_H

#include <ncurses.h>
#include <string>
#include <vector>

#include "focusable.h"
#include "pane.h"
#include "yate.h"

// Prompt window is a window that shows a list of options and a textbox with
// matching and "auto-complete"

class PromptWindow : public Pane, public Focusable {
  std::vector<size_t> get_matching_items() {
    std::vector<size_t> result;
    for (unsigned int i = 0; i < getListSize(); i++) {
      if (match(prompt_buffer, i)) result.push_back(i);
    }
    return result;
  }
  int highlighted_index = 0;

 protected:
  Yate& yate;
  bool input_enabled = true;
  std::string prompt_buffer;

 public:
  PromptWindow(Yate& yate)
      : Pane((Pane*)(yate.root), COLS / 4, LINES / 4, COLS / 2, LINES / 2),
        yate(yate) {}

  ~PromptWindow() { curs_set(1); }

  void onResize() {
    // PromptWindows have their own resize
    Pane::resize(COLS / 4, LINES / 4, COLS / 2, LINES / 2);
  }

  int capture() override {
    draw();
    curs_set(input_enabled);
    return mvwgetch(internal_window, 1, 1 + prompt_buffer.size());
  }

  void onKeyPress(int key) override {
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
        }
        break;
      case KEY_DOWN:
        highlighted_index++;
        break;
      case 27:  // Escape
        yate.exitPrompt();
        break;
      case KEY_ENTER:
      case 10:
      case 13:
        if (highlighted_index >= 0) {
          /* Calculate actual hovered item */
          std::vector<size_t> matched_items = get_matching_items();
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

  void draw() override {
    Logging::breadcrumb("PromptWindow Draw");
    wmove(internal_window, 1, 1);
    wclrtoeol(internal_window);
    mvwprintw(internal_window, 1, 1, prompt_buffer.c_str());

    std::vector<size_t> matched_items = get_matching_items();
    if (highlighted_index >= 0 &&
        (unsigned int)highlighted_index >= matched_items.size()) {
      highlighted_index = matched_items.size() - 1;
    }

    if (highlighted_index < 0 && !matched_items.empty()) {
      highlighted_index = 0;
    }

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
      if (str.length() < width - 2) {
        str.insert(str.end(), width - 2 - str.length(), ' ');
      }
      mvwinsstr(internal_window, print_row, 1, str.c_str());
      wattroff(internal_window, A_REVERSE);
      print_row++;
    }

    wmove(internal_window, 2, 1);
    whline(internal_window, 0, width - 2);
    wborder(internal_window, 0, 0, 0, 0, 0, 0, 0, 0);

    wattron(internal_window, A_DIM);
    mvwprintw(internal_window, 0, 1, getTitle().c_str());
    wattroff(internal_window, A_DIM);

    Logging::breadcrumb("PromptWindow Done");
    wrefresh(internal_window);
  }

  virtual const std::string& getTitle() override = 0;
  virtual bool match(std::string buffer, size_t index) = 0;
  virtual const std::string getItemString(size_t index) = 0;
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
