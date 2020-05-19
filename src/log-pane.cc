#include "log-pane.h"
#include "logging.h"

void LogPane::init() {
  Logging::addListener(this);
  getBuffer()->setSyntax("none");
  getBuffer()->setPfxTrieEnabled(false);
}

LogPane::~LogPane() {
  Logging::removeListener(this);
  delete getBuffer();
}

void LogPane::draw() {
  Buffer* buffer = getBuffer();
  // 00:00:00 = 8 length
  size_t field_width = 10;
  size_t start = 0;
  if (buffer->size() >= height - 1) {
    start = buffer->size() - height - 1;
  }
  size_t i = 0;
  for (size_t curr = start; curr < buffer->size(); curr++) {
    std::string& line = buffer->getLine(curr);

    // Right justify doesn't work.
    wattron(internal_window, A_DIM);
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
    mvwprintw(internal_window, i, 1, line.substr(0, field_width - 2).c_str());
    wattroff(internal_window, A_DIM);

    for (ColNumber j = field_width - 1; j < line.size(); j++) {
      mvwaddch(internal_window, i, j + 1, line.at(j));
    }
    i += 1;
  }
  for (; i < height; i++) {
    wmove(internal_window, i, 0);
    wclrtoeol(internal_window);
  }
  wrefresh(internal_window);
}

int LogPane::capture() {
  // capture at correct location
  draw();
  curs_set(0);
  int capture = mvwgetch(internal_window, 0, 0);
  yate.moveEditorToFront(this);
  return capture;
}

void LogPane::onKeyPress(int key) {
  ActionManager::get().runAction(key, yate, this);
}
