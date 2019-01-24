#include <ncurses.h>
#include <algorithm>
#include <iterator>

#include "command-prompt.h"
#include "editor.h"
#include "util.h"

static std::string tab_replace(std::string line, int tab_size) {
  std::string result;
  for (auto c : line) {
    if (c == '\t') {
      for (int i = 0; i < tab_size; i++) {
        result += ' ';
      }
    } else {
      result += c;
    }
  }
  return result;
}

// TODO(felixguo): Handle line wrapping?
void Editor::draw() {
  while (window_start > current_line) {
    window_start -= 1;
  }

  while (window_start + height <= current_line) {
    window_start += 1;
  }

  Logging::breadcrumb("Editor Draw");
  Logging::info << "Start: " << window_start << " Height: " << height
                << "Current: " << current_line << std::endl;
  unsigned int i = 0;
  int field_width = buffer->getLineNumberFieldWidth() + 1;
  for (auto line :
       buffer->getBufferWindow(window_start, window_start + height)) {
    line = tab_replace(line, yate.config.tab_size());
    // Right justify doesn't work.
    wattron(internal_window, A_DIM);
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
    std::string line_number = std::to_string(window_start + i + 1);
    int spacing = field_width - line_number.length();
    mvwprintw(internal_window, i, spacing, line_number.c_str());
    wattroff(internal_window, A_DIM);
    mvwprintw(internal_window, i, field_width + 1, line.c_str());
    i += 1;
  }
  for (; i < height; i++) {
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
  }
  Logging::breadcrumb("Editor Done");
  wrefresh(internal_window);
}

int Editor::capture() {
  // capture at correct location
  draw();
  curs_set(1);
  int line_number_width = buffer->getLineNumberFieldWidth() + 2;
  int col = current_col;
  std::string& line = buffer->getLine(current_line);
  for (uint i = 0; i < current_col; i++) {
    if (line.at(i) == '\t') {
      // Minus one because the tab character itself counts too
      col += yate.config.tab_size() - 1;
    }
  }
  return mvwgetch(internal_window, current_line - window_start,
                  col + line_number_width);
}

const std::string& Editor::getTitle() { return buffer->getFileName(); }

void Editor::onKeyPress(int key) {
  switch (key) {
    case KEY_ENTER:
    case '\n':
    case '\r':
      buffer->insertCharacter('\n', current_line, current_col);
      break;
    case KEY_STAB:
    case '\t':
      if (yate.config.indentation_style() == YateConfig::IndentationStyle::TAB) {
        buffer->insertCharacter('\t', current_line, current_col);
      } else {
        for (int i = 0; i < yate.config.tab_size(); i++) {
          buffer->insertCharacter(' ', current_line, current_col);
        }
      }
      break;
    case KEY_BACKSPACE:
    case 127:
      buffer->backspace(current_line, current_col);
      break;
    case KEY_DC:
      buffer->_delete(current_line, current_col);
      break;
    case ctrl('s'):
      buffer->writeToFile();
      break;
    case ctrl('p'): {
      CommandPromptWindow* p = new CommandPromptWindow(yate, this);
      Logging::info << p << std::endl;
      yate.enterPrompt(p);
      break;
    }
    case ctrl('z'):
      buffer->undo(current_line, current_col);
      break;
    case ctrl('y'):
      buffer->redo(current_line, current_col);
      break;
    case ctrl('o'):
      yate.enterPrompt(new FileSystemWindow(
          yate, this, ".",
          std::bind(&Editor::switchBuffer, this, std::placeholders::_1)));
      break;
    case KEY_LEFT:
      if (current_col != 0) {
        current_col--;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_RIGHT:
      if (current_col != buffer->getLineLength(current_line)) {
        current_col++;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_UP:
      if (current_line != 0) {
        current_line--;
        updateColWithPhantom();
      }
      break;
    case KEY_DOWN:
      if (current_line != buffer->size() - 1) {
        current_line++;
        updateColWithPhantom();
      }
      break;
    case KEY_HOME:
      if (current_col != 0) {
        current_col = 0;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_END:
      ColNumber end_col = buffer->getLineLength(current_line);
      if (current_col != end_col) {
        current_col = end_col;
        phantom_col_pos = current_col;
      }
      break;
  }
  if (std::isprint(key)) {
    buffer->insertCharacter(key, current_line, current_col);
  }
}

void Editor::updateColWithPhantom() {
  ColNumber end_col = buffer->getLineLength(current_line);
  if (current_col < phantom_col_pos) {
    current_col = phantom_col_pos;
  }
  if (current_col > end_col) {
    current_col = end_col;
  } else {
    phantom_col_pos = current_col;
  }
}
