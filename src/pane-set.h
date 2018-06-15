// A Pane Set is a set of panes arranged in some grid-like fashion.
// A Pane can be a Tab Set or an Editor.
#ifndef PANE_SET_H
#define PANE_SET_H

#include <vector>

#include "pane.h"

class PaneSet
{
	std::vector<Pane*> panes;
	// Store Focused Pane
public:
	PaneSet();

	// TODO(anyone): Created better interface for proper splitting.
	void addPane(Pane *pane);
	~PaneSet();
};

#endif
