// Represents a pane in a window.
#ifndef PANE_H
#define PANE_H

#include <string>
#include <ncurses.h>

struct Pane
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	std::string title;

	virtual void draw(WINDOW *window) = 0;
	virtual ~Pane() = 0;
};

Pane::~Pane() { }

#endif
