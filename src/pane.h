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
	WINDOW *internal_window;

	virtual void draw() = 0;
	Pane()
	{
		height = LINES;
		width = COLS;
		internal_window = newwin(height, width, y, x);
		box(internal_window, 0, 0);
		refresh();
	}

	virtual ~Pane()
	{
		delwin(internal_window);
	}
};

#endif
