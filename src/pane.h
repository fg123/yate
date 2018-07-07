// Represents a pane in a window.
#ifndef PANE_H
#define PANE_H

#include <string>
#include <ncurses.h>

#include "src/config.pb.h"

using uint = unsigned int;

struct Pane
{
	uint x;
	uint y;
	uint width;
	uint height;
	WINDOW *internal_window;
	Pane *parent;

	virtual void draw() = 0;
	void resize(uint nx, uint ny, uint nwidth, uint nheight) {
		onResize(nx, ny, nwidth, nheight);
		x = nx;
		y = ny;
		width = nwidth;
		height = nheight;
	}
	// onResize should be called before updated, so we can do comparison
	virtual void onResize(uint nx, uint ny, uint nwidth, uint nheight) { }
	virtual const std::string &getTitle() = 0;
	Pane(Pane *parent, const YateConfig_State_Pane &fromConfig)
		: Pane(parent,
			   fromConfig.x(), fromConfig.y(), fromConfig.width(),
			   fromConfig.height())
	{
	}
	Pane(Pane *parent, int x, int y, int width, int height) : x(x), y(y), width(width), height(height), parent(parent)
	{
		internal_window = newwin(height, width, y, x);
		keypad(internal_window, true);
	}

	void titleUpdated()
	{
		onTitleUpdated();
		if (parent)
			parent->titleUpdated();
	}
	virtual void onTitleUpdated() {}
	virtual std::ostream &serialize(std::ostream &stream)
	{
		return stream;
	}
	virtual ~Pane()
	{
		delwin(internal_window);
	}
};

#endif
