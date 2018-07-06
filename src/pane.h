// Represents a pane in a window.
#ifndef PANE_H
#define PANE_H

#include <string>
#include <ncurses.h>

#include "src/config.pb.h"

struct Pane
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	WINDOW *internal_window;
	Pane *parent;

	virtual void draw() = 0;
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
