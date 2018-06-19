// Represents a pane in a window.
#ifndef PANE_H
#define PANE_H

#include <string>
#include <ncurses.h>

struct Pane
{
	int x;
	int y;
	int width;
	int height;
	std::string title;
	WINDOW *internal_window;

	virtual void draw() = 0;
	Pane(int x, int y, int width, int height) : x(x), y(y),
		width(width), height(height)
	{
		internal_window = newwin(height, width, y, x);
		// wborder(internal_window, 0, 0, 0, 0, 0, 0, 0, 0);
		refresh();
	}

	virtual ~Pane()
	{
		delwin(internal_window);
	}
};

#endif
