// Represents a tabbed set.
#ifndef TAB_SET_H
#define TAB_SET_H

#include <vector>
#include <ncurses.h>

#include "pane-set.h"
#include "pane.h"

class TabSet: public Pane
{
	std::vector<PaneSet*> tabs;
public:
	TabSet();
	~TabSet();
	void draw(WINDOW *window) override;
};

#endif
