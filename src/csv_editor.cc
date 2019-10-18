#include "csv_editor.h"

std::string CsvEditor::generateStatusBar() {
  std::ostringstream output;
  output << current_line + 1 << "/" << buffer->size() << "L " << current_col + 1
         << "C (" << buffer->getFileName()  << ")";
  return output.str();
}

void CsvEditor::populate_widths(BufferWindow window) {
  for (auto line : window) {

  }
}

void CsvEditor::draw() {
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

  for (auto line_it = content_window.begin();
       line_it != content_window.end();
       line_it++) {
    std::string line =
        tab_replace(*line_it, *line_it, yate.config.getTabSize());
    if (window_start_col > line.size()) {
      line = "";
    } else {
      line = line.substr(window_start_col, width - field_width);
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
      mvwaddch(internal_window, i, field_width + 1 + j, line.at(j) | flag);
    }
    i += 1;
  }
  for (; i < height - 1; i++) {
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
  }
  std::string bottom_bar = "CSV " + generateStatusBar();
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

int CsvEditor::capture() {
  // capture at correct location
  draw();
  curs_set(1);
  int line_number_width = buffer->getLineNumberFieldWidth() + 2;
  int col = getActualColPosition();
  int capture = mvwgetch(internal_window, current_line - window_start_line,
                         col + line_number_width - window_start_col);
  yate.moveEditorToFront(this);
  return capture;
}
