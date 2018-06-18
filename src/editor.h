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

class Editor: public Pane {
	std::vector<std::string> buffer;
	std::string path;
	unsigned int line;

public:
	Editor(std::string path) : path(path) {
		std::ifstream file(path);
		std::copy(std::istream_iterator<std::string>(file),
		  std::istream_iterator<std::string>(),
		  std::back_inserter(buffer));
		height = LINES;
	}

	void draw() override {
		Logging::info("Editor Draw " + std::to_string(height));
		for (int i = 0; i < buffer.size(); i++) {
			Logging::info("Editor: " + buffer.at(i));
			mvwprintw(internal_window, i, 0, "%s", buffer.at(i).c_str());
		}
		Logging::info("Editor Done");
		wrefresh(internal_window);
		refresh();
	}
};

#endif
