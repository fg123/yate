#ifndef PROMPT_WINDOW_H
#define PROMPT_WINDOW_H

#include <ncurses.h>
#include <vector>
#include <string>

#include "pane.h"
#include "focusable.h"
#include "yate.h"

// Prompt window is a window that shows a list of options and a textbox with
// matching and "auto-complete"

template <class T>
class PromptWindow : public Pane, public Focusable {
	Yate &yate;
	std::string prompt_buffer;
	std::vector<T> items;
	int highlighted_index = 0;
public:
	PromptWindow(Yate &yate)
	: Pane((Pane*)(yate.root), COLS / 4, LINES / 4, COLS / 2, LINES / 2),
	yate(yate) {}

	int capture() override {
		draw();
		return mvwgetch(internal_window, 1, 1 + prompt_buffer.size());
	}

	void onKeyPress(int key) override {
		switch(key) {
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
			// if (highlighted_index > 0) {
			// 	highlighted_index--;
			// }
			break;
		case 27: // Escape
		case KEY_ENTER:
		case 10:
		case 13:
			yate.exitPrompt();
			break;
		}
		if (std::isprint(key)) {
			prompt_buffer.push_back(static_cast<char>(key));
		}
	}

	void draw() override {
		Logging::breadcrumb("PromptWindow Draw");
		wmove(internal_window, 1, 1);
		wclrtoeol(internal_window);
		mvwprintw(internal_window, 1, 1, prompt_buffer.c_str());


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
	virtual bool match(T item) = 0;
};
#endif
