// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <ncurses.h>
#include <algorithm>

#include "logging.h"
#include "pane.h"
#include "buffer.h"

class Editor: public Pane {
	Buffer *buffer;
	unsigned int line;

public:
	Editor(Buffer *buffer) : buffer(buffer) {
	}

	void draw() override {
		Logging::info("Editor Draw " + std::to_string(height));
		int i = 1;
		for (auto line : (buffer->getBufferWindow(0, buffer->size()))) {
			Logging::info("Editor: " + line);
			//mvwprintw(internal_window, i, 1, "%s", line.c_str());
			i += 1;
		}
		Logging::info("Editor Done");
		wrefresh(internal_window);
		refresh();
		wgetch(internal_window);
	}
};

#endif
