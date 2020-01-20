#include <ncurses.h>
#include <algorithm>
#include <iterator>

#include "command-prompt.h"
#include "editor.h"
#include "filesystem-prompt.h"
#include "find-all-prompt.h"
#include "find-prompt.h"
#include "goto-line-prompt.h"
#include "resize-prompt.h"
#include "tab-set.h"
#include "tags-prompt.h"
#include "util.h"

/* TODO(felixguo): find cross terminal for this */
/* https://gist.github.com/rkumar/1237091 */
#define KEY_SUP 337
#define KEY_SDOWN 336

#define KEY_CLEFT 553
#define KEY_CRIGHT 568
#define KEY_ESC 27

void Editor::init() {
  current_node = nullptr;
  buffer->registerEditor(this);
  yate.registerEditor(this);
  std::vector<Pane*> parents;
  paneset_parent = findFirstParent<PaneSet>(parents);
  if (!parents.empty()) {
    paneset_parent_child = parents.back();
  }
  limitLineCol();
}

Editor::~Editor() {
  yate.unregisterEditor(this);
  buffer->unregisterEditor(this);
}

void Editor::revertBuffer() {
  buffer->revert(current_line, current_col);
  limitLineCol();
}

std::string Editor::generateStatusBar() {
  std::ostringstream output;
  output << current_line + 1 << "/" << buffer->size() << "L " << current_col + 1
         << "C " << window_start_line << "SL " << window_start_col << "SC "
         << yate.config.getIndentationStyle() << ": "
         << yate.config.getTabSize() << " (" << buffer->getFileName() << ":"
         << current_word << (is_at_end_of_word ? ">" : "") << ")";
  return output.str();
}

bool Editor::onNavigationItemSelected(size_t index, NavigateWindow* parent) {
  switch (index) {
    case 0:
      focusRequested(this);
      break;
    case 1:
      paneset_parent->verticalSplit(paneset_parent_child);
      break;
    case 2:
      paneset_parent->horizontalSplit(paneset_parent_child);
      break;
    case 3:
      paneset_parent->mergePane(paneset_parent_child, parent);
      // MergePane could spawn another prompt window, so we return
      //   false here so the navigate window tree stays intact.
      // It is the responsibility of mergePane to call finish on
      //   the active navigate window.
      return false;
    case 4:
      ResizePromptWindow* prompt =
          new ResizePromptWindow(yate, paneset_parent, paneset_parent_child);
      parent->finish();
      yate.enterPrompt(prompt);
      return false;
  }
  return true;
}

bool Editor::inSelection(LineNumber line, ColNumber col) {
  if (selection_start == NO_SELECTION) {
    return false;
  }
  // Selection Start is based on buffer position, count how many tabs before
  //   to get screen location
  std::string& line_content = buffer->getLine(std::get<0>(selection_start));
  size_t tab_count = 0;
  for (size_t i = 0; i < std::get<1>(selection_start); i++) {
    if (line_content[i] == '\t') tab_count += 1;
  }
  LineCol modified_selection_start = selection_start;
  std::get<1>(modified_selection_start) +=
      tab_count * (yate.config.getTabSize() - 1);
  LineCol location = std::make_tuple(line, col);
  LineCol cursor = std::make_tuple(current_line, getActualColPosition());
  LineCol from = std::min(cursor, modified_selection_start);
  LineCol to = std::max(cursor, modified_selection_start);
  return location >= from && location <= to;
}

ColNumber Editor::getActualColPosition() {
  std::string& line = buffer->getLine(current_line);
  int tab_size = yate.config.getTabSize();
  uint m = 0;
  for (uint i = 0; i < current_col; i++, m++) {
    if (line.at(i) == '\t') {
      m += (tab_size * ((m / tab_size) + 1) - m) - 1;
    }
  }
  return m;
}

// TODO(felixguo): Handle line wrapping?
void Editor::draw() {
  unsigned int i = 0;
  int field_width = buffer->getLineNumberFieldWidth() + 1;

  if (window_start_line > current_line) {
    window_start_line = current_line;
  }

  if (window_start_line + height - 2 < current_line) {
    window_start_line = current_line - (height - 2);
  }

  ColNumber actual_col = getActualColPosition();
  if (window_start_col > actual_col) {
    window_start_col = actual_col;
  }

  if (window_start_col + width - field_width - 2 < actual_col) {
    window_start_col = actual_col - (width - field_width - 2);
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

    std::unordered_set<int> markers = yate.config.getColumnMarkers();

    // Calculate if it's part of a selection
    auto empty_marker = yate.config.getTheme()->map(SyntaxHighlighting::Component::NO_HIGHLIGHT_MARKER);
    for (ColNumber j = 0; j < line.size(); j++) {
      auto flag = inSelection(window_start_line + i, window_start_col + j)
                      ? A_REVERSE
                      : A_NORMAL;
      int highlight = syntax.at(j);
      bool is_marker = false;
      if (markers.find(window_start_col + j) != markers.end()) {
        is_marker = true;
        // highlight += 1;
        markers.erase(window_start_col + j);
        flag = A_DIM | A_REVERSE;
      }

      auto syntax_color = yate.config.getTheme()->map((SyntaxHighlighting::Component) highlight);

      if (yate.should_highlight) wattron(internal_window, syntax_color);
      // else if (is_marker) wattron(internal_window, empty_marker);

      mvwaddch(internal_window, i, field_width + 1 + j, line.at(j) | flag);

      if (yate.should_highlight) wattroff(internal_window, syntax_color);
      // else if (is_marker) wattroff(internal_window, empty_marker);
    }

    for (int m : markers) {
      // wattron(internal_window, empty_marker);
      mvwaddch(internal_window, i, field_width + 1 + m - window_start_col, ' ' | A_DIM | A_REVERSE);
       // wattroff(internal_window, empty_marker);
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
  size_t maxlen = 0;
  for (auto str : suggested_complete) {
    maxlen = std::max(str.size() + 3, maxlen);
  }
  for (ColNumber i = 0; i < suggested_complete.size(); i++) {
    ColNumber j = 0;
    std::string line = (i == suggested_complete_index ? "> " : "  ") +
                       suggested_complete[i] + " ";
    for (; j < line.size(); j++) {
      mvwaddch(internal_window, current_line + 1 + i - window_start_line,
               field_width + 1 + current_col + 1 + j - window_start_col,
               line[j] | A_REVERSE);
    }
    for (; j < maxlen; j++) {
      mvwaddch(internal_window, current_line + 1 + i - window_start_line,
               field_width + 1 + current_col + 1 + j - window_start_col,
               ' ' | A_REVERSE);
    }
  }
  wrefresh(internal_window);
}

int Editor::capture() {
  // capture at correct location
  curs_set(0);
  draw();
  int line_number_width = buffer->getLineNumberFieldWidth() + 2;
  int col = getActualColPosition();
  curs_set(1);
  int capture = mvwgetch(internal_window, current_line - window_start_line,
                         col + line_number_width - window_start_col);
  curs_set(0);
  yate.moveEditorToFront(this);
  return capture;
}

const std::string& Editor::getTitle() { return buffer->getFileName(); }

void Editor::goToLineOffset(int offset, bool shouldMoveLineToCenter) {
  LineNumber go = current_line + offset;
  if (offset < 0 && current_line <= (unsigned int)(-offset)) {
    go = 0;
  }
  goToLine(go, shouldMoveLineToCenter);
}

void Editor::goToLine(LineNumber n, bool shouldMoveLineToCenter) {
  current_line = n;
  limitLineCol();

  if (shouldMoveLineToCenter) {
    int ideal_start = current_line - (height / 2);
    window_start_line = std::max(ideal_start, 0);
  }
}

void Editor::goToLineCol(LineNumber l, ColNumber c,
                         bool shouldMoveLineToCenter) {
  current_col = c;
  goToLine(l, shouldMoveLineToCenter);

  if (shouldMoveLineToCenter) {
    int ideal_start = current_col - (width / 2);
    window_start_col = std::max(ideal_start, 0);
  }
}

void Editor::insertTab(LineNumber& line, ColNumber& col) {
  if (yate.config.getIndentationStyle() == YateConfig::IndentationStyle::TAB) {
    buffer->insertCharacter('\t', line, col);
  } else {
    for (int i = 0; i < yate.config.getTabSize(); i++) {
      buffer->insertCharacter(' ', line, col);
    }
  }
}

void Editor::removeTab(LineNumber& line, ColNumber& col) {
  std::string& _line = buffer->getLine(line);
  ColNumber zero = 0;
  if (yate.config.getIndentationStyle() == YateConfig::IndentationStyle::TAB) {
    if (_line.size() > 0 && _line[0] == '\t') {
      buffer->_delete(line, zero);
    }
  } else {
    if (startsWith(std::string(yate.config.getTabSize(), ' '), _line)) {
      for (int i = 0; i < yate.config.getTabSize(); i++) {
        buffer->_delete(line, zero);
        zero = 0;
      }
    }
  }
}

void Editor::addTag(std::string label) {
  buffer->addTag(label, current_line, current_col);
}

void Editor::fastTravel(EditNode* to) {
  buffer->fastTravel(to, current_line, current_col);
}

void Editor::deleteWord() {
  buffer->deleteWord(current_line, current_col);
  limitLineCol();
}

void Editor::deleteLine() {
  buffer->deleteLine(current_line);
  limitLineCol();
}

void Editor::deleteSelection() {
  if (selection_start == NO_SELECTION) return;
  LineCol current = std::make_tuple(current_line, current_col);
  buffer->deleteRange(current, selection_start);
  current = std::min(selection_start, current);
  current_line = std::get<0>(current);
  current_col = std::get<1>(current);
  selection_start = NO_SELECTION;
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
    if (key == KEY_SLEFT && current_col == 0) {
      onKeyPress(KEY_SUP);
      key = KEY_SEND;
    }
    if (key == KEY_SRIGHT &&
        current_col == buffer->getLineLength(current_line)) {
      onKeyPress(KEY_SDOWN);
      key = KEY_SHOME;
    }
  }
  ActionManager::get().runAction(key, yate, this);
  switch (key) {
    case '\r':
    case '\n':
    case KEY_ENTER: {
      if (shouldShowAutoComplete()) {
        std::string insert =
            suggested_complete[suggested_complete_index].substr(
                current_word.size());
        buffer->insertString(insert, current_line, current_col);
        break;
      }
      if (selection_start != NO_SELECTION) {
        // Delete selection if we type during selection
        deleteSelection();
      }
      buffer->insertCharacter('\n', current_line, current_col);
      if (!buffer->isInPasteMode) {
        std::string& prev_line = buffer->getLine(current_line - 1);
        ColNumber end = prev_line.find_first_not_of(" \t");
        for (ColNumber i = 0; i < std::min(end, prev_line.size()); i++) {
          buffer->insertCharacter(prev_line[i], current_line, current_col);
        }
      }
      break;
    }
    case KEY_BACKSPACE:
      if (selection_start == NO_SELECTION) {
        buffer->backspace(current_line, current_col);
      } else {
        deleteSelection();
      }
      break;
    case KEY_DC:
      if (selection_start == NO_SELECTION) {
        buffer->_delete(current_line, current_col);
      } else {
        deleteSelection();
      }
      break;
    case KEY_ESC:
      suggested_complete.clear();
      break;
    case ctrl('w'): {
      return;
    }
    case KEY_CLEFT: {
      TabSet* first = findFirstParent<TabSet>();
      if (first) {
        first->prevTab();
      }
      break;
    }
    case KEY_CRIGHT: {
      TabSet* first = findFirstParent<TabSet>();
      if (first) {
        first->nextTab();
      }
      break;
    }
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
      if (shouldShowAutoComplete()) {
        if (suggested_complete_index == 0) {
          suggested_complete_index = suggested_complete.size();
        }
        suggested_complete_index -= 1;
        break;
      }
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
      if (shouldShowAutoComplete()) {
        suggested_complete_index += 1;
        if (suggested_complete_index == suggested_complete.size()) {
          suggested_complete_index = 0;
        }
        break;
      }
      if (current_line != buffer->size() - 1) {
        current_line++;
        updateColWithPhantom();
      } else {
        current_col = buffer->getLineLength(current_line);
        phantom_col_pos = current_col;
      }
      break;
    case KEY_SHOME:
    case KEY_HOME: {
      ColNumber desired_col =
          buffer->getLine(current_line).find_first_not_of(" \t");
      if (current_col <= desired_col) {
        desired_col = 0;
      }
      if (current_col != desired_col) {
        current_col = desired_col;
        phantom_col_pos = current_col;
      }
      break;
    }
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
    if (selection_start != NO_SELECTION) {
      deleteSelection();
    }
    buffer->insertCharacter(key, current_line, current_col);
  }
  limitLineCol();
  if (is_at_end_of_word && current_node &&
      (shouldShowAutoComplete() || std::isprint(key))) {
    suggested_complete = current_node->getSuffixes();
    if (suggested_complete.size() > 0 && suggested_complete[0].size() == 1) {
      suggested_complete.erase(suggested_complete.begin());
    }
    if (suggested_complete_index >= suggested_complete.size()) {
      suggested_complete_index = suggested_complete.size() - 1;
    }
    for (auto& str : suggested_complete) {
      str.insert(0, current_word, 0, current_word.size() - 1);
    }
  } else {
    suggested_complete.clear();
  }
}

bool Editor::shouldShowAutoComplete() {
  return is_at_end_of_word && current_node && suggested_complete.size() > 0 &&
         !buffer->isInPasteMode;
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

void Editor::invalidate_current_word() {
  current_word =
      buffer->getWordAt(current_line, current_col, &is_at_end_of_word);
  current_node = buffer->prefix_trie.getNode(current_word);
}

// This should be called whenever anything happens to the current
//   line or col
void Editor::limitLineCol() {
  if (current_line < 0) current_line = 0;
  if (current_line >= buffer->size()) {
    current_line = buffer->size() - 1;
  }
  if (current_col < 0) current_col = 0;
  if (buffer->getLineLength(current_line) == 0) {
    /* Don't go to case below, since that will underflow
       I believe this is OK with the rows because there
       cannot be a empty file with 0 rows */
    current_col = 0;
  } else if (current_col >= buffer->getLineLength(current_line)) {
    current_col = buffer->getLineLength(current_line);
  }

  if (current_word_line != current_line || current_col != current_word_col) {
    // Invalidate Current Word for SURE
    invalidate_current_word();
  }
  current_word_line = current_line;
  current_word_col = current_col;
}

void Editor::switchBuffer(std::string newPath) {
  switchBuffer(yate.getBuffer(newPath));
}

void Editor::switchBuffer(Buffer* newBuffer) {
  buffer->unregisterEditor(this);
  buffer = newBuffer;
  buffer->registerEditor(this);
  titleUpdated();
  limitLineCol();
}

void Editor::paste(std::string& str) {
  deleteSelection();
  buffer->insertString(str, current_line, current_col);
}

void Editor::onMouseEvent(MEVENT* event) {
  if (event->bstate & BUTTON1_PRESSED) {
    if (!yate.isCurrentFocus(this)) {
      focusRequested(this);
    } else {
      selection_start = NO_SELECTION;

      current_line = (event->y - y) + window_start_line;
      current_col = (event->x - x) - (buffer->getLineNumberFieldWidth() + 2) +
                    window_start_col;
      /* Count tabs before click point */
      limitLineCol();
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
      limitLineCol();
    }
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
