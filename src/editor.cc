#include <iterator>
#include <ncurses.h>
#include <algorithm>

#include "editor.h"
#include "util.h"
#include "command-prompt.h"

void Editor::draw() {
	Logging::breadcrumb("Editor Draw");
	unsigned int i = 0;
	int field_width = buffer->getLineNumberFieldWidth() + 1;
	for (auto line : buffer->getBufferWindow(window_start, window_start + height)) {
		// Right justify doesn't work.
		wattron(internal_window, A_DIM);
		wmove(internal_window, i, 0);
		wclrtoeol(internal_window);
		std::string line_number = std::to_string(window_start + i + 1);
		int spacing = field_width - line_number.length();
		mvwprintw(internal_window, i, spacing, line_number.c_str());
		wattroff(internal_window, A_DIM);
		// if (window_start + i == current_line) {
		// 	wattron(internal_window, A_UNDERLINE);
		// }
		mvwprintw(internal_window, i, field_width + 1,
			line.c_str());
		// wattroff(internal_window, A_UNDERLINE);
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
	int line_number_width = buffer->getLineNumberFieldWidth() + 2;
	return mvwgetch(internal_window, current_line, current_col + line_number_width);
}

const std::string& Editor::getTitle() {
	return buffer->getFileName();
}

void Editor::onKeyPress(int key) {
	switch(key) {
	case KEY_ENTER:
	case '\n':
	case '\r':
		buffer->insertCharacter('\n', current_line, current_col);
		break;
	case KEY_STAB:
	case '\t':
		// TODO (felixguo): Add proper handling of tabs
		buffer->insertCharacter(' ', current_line, current_col);
		buffer->insertCharacter(' ', current_line, current_col);
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
		CommandPromptWindow *p = new CommandPromptWindow(yate);
		Logging::info << p << "\n";
		yate.enterPrompt(p);
		break;
	}
	case ctrl('z'):
		buffer->undo(current_line, current_col);
		break;
	case ctrl('y'):
		buffer->redo(current_line, current_col);
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
	}
	else {
		phantom_col_pos = current_col;
	}
}
