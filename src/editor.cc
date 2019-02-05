#include <ncurses.h>
#include <algorithm>
#include <iterator>

#include "command-prompt.h"
#include "editor.h"
#include "filesystem-prompt.h"
#include "find-prompt.h"
#include "goto-line-prompt.h"
#include "util.h"

/* TODO(felixguo): find cross terminal for this */
/* https://gist.github.com/rkumar/1237091 */
#define KEY_SUP 337
#define KEY_SDOWN 336

static std::string tab_replace(std::string& line, std::string& reference,
                               int tab_size, char replace_with = ' ') {
  std::string result;
  for (ColNumber i = 0; i < line.size(); i++) {
    char c = reference[i];
    if (c == '\t') {
      for (int i = 0; i < tab_size; i++) {
        result += replace_with;
      }
    } else {
      result += line[i];
    }
  }
  return result;
}

void Editor::init() { buffer->registerEditor(this); }

void Editor::revertBuffer() {
  buffer->revert(current_line, current_col);
  limitLine();
  limitCol();
}

std::string Editor::generateStatusBar() {
  std::ostringstream output;
  output << current_line + 1 << "L " << current_col + 1 << "C "
         << window_start_line << "SL " << window_start_col << "SC "
         << yate.config.getIndentationStyle() << ": "
         << yate.config.getTabSize();
  return output.str();
}

bool Editor::inSelection(LineNumber line, ColNumber col) {
  if (selection_start == NO_SELECTION) {
    return false;
  }
  LineCol location = std::make_tuple(line, col);
  LineCol cursor = std::make_tuple(current_line, current_col);
  LineCol from = std::min(cursor, selection_start);
  LineCol to = std::max(cursor, selection_start);
  return location >= from && location <= to;
}

// TODO(felixguo): Handle line wrapping?
void Editor::draw() {
  Logging::breadcrumb("Editor Draw");
  unsigned int i = 0;
  int field_width = buffer->getLineNumberFieldWidth() + 1;

  if (window_start_line > current_line) {
    window_start_line = current_line;
  }

  if (window_start_line + height - 2 < current_line) {
    window_start_line = current_line - (height - 2);
  }

  if (window_start_col > current_col) {
    window_start_col = current_col;
  }

  if (window_start_col + width - field_width - 2 < current_col) {
    window_start_col = current_col - (width - field_width - 2);
  }
  BufferWindow content_window = buffer->getBufferWindow(
      window_start_line, window_start_line + height - 1);
  BufferWindow syntax_window = buffer->getSyntaxBufferWindow(
      window_start_line, window_start_line + height - 1);
  for (auto line_it = content_window.begin(), syntax_it = syntax_window.begin();
       line_it != content_window.end() && syntax_it != syntax_window.end();
       line_it++, syntax_it++) {
    std::string line =
        tab_replace(*line_it, *line_it, yate.config.getTabSize());
    std::string syntax =
        tab_replace(*syntax_it, *line_it, yate.config.getTabSize(),
                    (char)SyntaxHighlighting::Component::WHITESPACE);
    if (window_start_col > line.size()) {
      line = "";
    } else {
      line = line.substr(window_start_col, width - field_width);
      syntax = syntax.substr(window_start_col, width - field_width);
    }
    // Right justify doesn't work.
    wattron(internal_window, A_DIM);
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
    std::string line_number = std::to_string(window_start_line + i + 1);
    int spacing = field_width - line_number.length();
    mvwprintw(internal_window, i, spacing, line_number.c_str());
    wattroff(internal_window, A_DIM);

    // Calculate if it's part of a selection
    for (ColNumber j = 0; j < line.size(); j++) {
      auto flag = inSelection(window_start_line + i, window_start_col + j)
                      ? A_REVERSE
                      : A_NORMAL;
      auto syntax_color = yate.config.getTheme()->map(
          (SyntaxHighlighting::Component)syntax.at(j));
      init_pair(4, 64, -1);
      wattron(internal_window, syntax_color);
      mvwaddch(internal_window, i, field_width + 1 + j, line.at(j) | flag);
      wattroff(internal_window, syntax_color);
    }
    i += 1;
  }
  for (; i < height - 1; i++) {
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
  }
  std::string bottom_bar = generateStatusBar();
  for (unsigned int i = 0; i < width; i++) {
    char draw = i < bottom_bar.size() ? bottom_bar[i] : ' ';
    mvwaddch(internal_window, height - 1, i, draw | A_REVERSE);
  }
  Logging::breadcrumb("Editor Done");
  wrefresh(internal_window);
}

int Editor::capture() {
  Logging::breadcrumb("Editor Capture Called");
  // capture at correct location
  draw();
  curs_set(1);
  int line_number_width = buffer->getLineNumberFieldWidth() + 2;
  int col = current_col;
  std::string& line = buffer->getLine(current_line);
  for (uint i = 0; i < current_col; i++) {
    if (line.at(i) == '\t') {
      // Minus one because the tab character itself counts too
      col += yate.config.getTabSize() - 1;
    }
  }
  return mvwgetch(internal_window, current_line - window_start_line,
                  col + line_number_width - window_start_col);
}

const std::string& Editor::getTitle() { return buffer->getFileName(); }

void Editor::goToLine(LineNumber n) {
  current_line = n;
  limitLine();
  limitCol();
}

void Editor::onKeyPress(int key) {
  if (key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT ||
      key == KEY_HOME || key == KEY_END) {
    selection_start = NO_SELECTION;
  } else if (key == KEY_SUP || key == KEY_SDOWN || key == KEY_SLEFT ||
             key == KEY_SRIGHT || key == KEY_SHOME || key == KEY_SEND) {
    if (selection_start == NO_SELECTION) {
      selection_start = std::make_tuple(current_line, current_col);
    }
  }
  switch (key) {
    case KEY_ENTER:
    case '\n':
    case '\r':
      buffer->insertCharacter('\n', current_line, current_col);
      break;
    case KEY_STAB:
    case '\t':
      if (yate.config.getIndentationStyle() ==
          YateConfig::IndentationStyle::TAB) {
        buffer->insertCharacter('\t', current_line, current_col);
      } else {
        for (int i = 0; i < yate.config.getTabSize(); i++) {
          buffer->insertCharacter(' ', current_line, current_col);
        }
      }
      break;
    case KEY_BACKSPACE:
    case 127:
      if (selection_start == NO_SELECTION) {
        buffer->backspace(current_line, current_col);
      } else {
        LineCol current = std::make_tuple(current_line, current_col);
        buffer->deleteRange(current, selection_start);
        current = std::min(selection_start, current);
        current_line = std::get<0>(current);
        current_col = std::get<1>(current);
        selection_start = NO_SELECTION;
      }
      break;
    case KEY_DC:
      if (selection_start == NO_SELECTION) {
        buffer->_delete(current_line, current_col);
      } else {
        LineCol current = std::make_tuple(current_line, current_col);
        buffer->deleteRange(current, selection_start);
        current = std::min(selection_start, current);
        current_line = std::get<0>(current);
        current_col = std::get<1>(current);
        selection_start = NO_SELECTION;
      }
      break;
    case ctrl('f'): {
      FindPromptWindow* f = new FindPromptWindow(yate, this);
      yate.enterPrompt(f);
    }
    case ctrl('s'):
      buffer->writeToFile();
      break;
    case ctrl('p'): {
      CommandPromptWindow* p = new CommandPromptWindow(yate, this);
      yate.enterPrompt(p);
      break;
    }
    case ctrl('g'): {
      GoToLinePromptWindow* p = new GoToLinePromptWindow(yate, this);
      yate.enterPrompt(p);
      break;
    }
    case ctrl('z'):
      buffer->undo(current_line, current_col);
      selection_start = NO_SELECTION;
      break;
    case ctrl('a'):
      selection_start = std::make_tuple(0, 0);
      current_line = buffer->size() - 1;
      current_col = buffer->getLineLength(current_line);
      break;
    case ctrl('y'):
      buffer->redo(current_line, current_col);
      selection_start = NO_SELECTION;
      break;
    case ctrl('o'):
      yate.enterPrompt(new FileSystemWindow(
          yate, this, ".",
          std::bind(
              static_cast<void (Editor::*)(std::string)>(&Editor::switchBuffer),
              this, std::placeholders::_1)));
      break;
    case KEY_LEFT:
    case KEY_SLEFT:
      if (current_col != 0) {
        current_col--;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_RIGHT:
    case KEY_SRIGHT:
      if (current_col != buffer->getLineLength(current_line)) {
        current_col++;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_UP:
    case KEY_SUP:
      if (current_line != 0) {
        current_line--;
        updateColWithPhantom();
      } else {
        current_col = 0;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_DOWN:
    case KEY_SDOWN:
      if (current_line != buffer->size() - 1) {
        current_line++;
        updateColWithPhantom();
      } else {
        current_col = buffer->getLineLength(current_line);
        phantom_col_pos = current_col;
      }
      break;
    case KEY_SHOME:
    case KEY_HOME:
      if (current_col != 0) {
        current_col = 0;
        phantom_col_pos = current_col;
      }
      break;
    case KEY_SEND:
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
  limitLine();
  limitCol();
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

void Editor::limitLine() {
  if (current_line < 0) current_line = 0;
  if (current_line >= buffer->size()) {
    current_line = buffer->size() - 1;
  }
}

void Editor::limitCol() {
  if (current_col < 0) current_col = 0;
  if (buffer->getLineLength(current_line) == 0) {
    /* Don't go to case below, since that will underflow
       I believe this is OK with the rows because there
       cannot be a empty file with 0 rows */
    current_col = 0;
  } else if (current_col >= buffer->getLineLength(current_line)) {
    current_col = buffer->getLineLength(current_line);
  }
}

void Editor::switchBuffer(std::string newPath) {
  switchBuffer(yate.getBuffer(newPath));
}

void Editor::switchBuffer(Buffer* newBuffer) {
  buffer->unregisterEditor(this);
  buffer = newBuffer;
  buffer->registerEditor(this);
  titleUpdated();
  limitLine();
  limitCol();
}

void Editor::onMouseEvent(MEVENT* event) {
  if (event->bstate & BUTTON1_PRESSED) {
    if (!yate.isCurrentFocus(this)) {
      focusRequested(this);
    }
    current_line = (event->y - y) + window_start_line;
    current_col = (event->x - x) - (buffer->getLineNumberFieldWidth() + 2) +
                  window_start_col;
    /* Count tabs before click point */
    limitLine();
    std::string line = buffer->getLine(current_line);
    for (ColNumber i = 0; i < std::min(current_col, line.size()); i++) {
      if (line.at(i) == '\t') {
        if (current_col < i + yate.config.getTabSize()) {
          current_col = i;
        } else {
          current_col -= (yate.config.getTabSize() - 1);
        }
      }
    }
    limitCol();
  }
  // } else if (event->bstate & BUTTON4_PRESSED) {
  //   Logging::info << "Here" << std::endl;
  //   window_start_line -= 1;
  //   current_line += 1;
  //   limitLine();
  // } else if (event->bstate & BUTTON5_PRESSED) {
  //   // TODO(anyone): If we can use ncurses6, they have better support
  //   window_start_line += 1;
  //   current_line -= 1;
  //   limitLine();
  // }
}
