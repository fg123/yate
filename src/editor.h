// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <ncurses.h>

#include "pane.h"

class Editor: public Pane
{
	std::vector<std::string> buffer;
	std::string path;
	unsigned int line;

public:
	Editor(std::string path) : path(path) {}
	void draw(WINDOW *window) override {}
};

#endif
