#include <iterator>
#include <ncurses.h>
#include <algorithm>

#include "editor.h"

void Editor::draw() {
	Logging::info("Editor Draw " + std::to_string(height));
	int i = 0;
	int field_width = buffer->getLineNumberFieldWidth() + 1;

	for (auto line : (buffer->getBufferWindow(window_start, window_start + height))) {
		Logging::info("Editor: " + std::string(line.c_str()));
		// Right justify doesn't work.
		wattron(internal_window, A_DIM);
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
	Logging::info("Editor Done");
	refresh();
	wrefresh(internal_window);
}

void Editor::focus() {

}
