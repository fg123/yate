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
	unsigned int selected_tab;
public:
	TabSet(int x, int y, int width, int height);
	~TabSet();
	void draw() override;
};

#endif
