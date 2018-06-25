// A Pane Set is a set of panes arranged in some grid-like fashion.
// A Pane can be a Tab Set or an Editor.
#ifndef PANE_SET_H
#define PANE_SET_H

#include <vector>

#include "logging.h"
#include "pane.h"
#include "yate.h"

class PaneSet : public Pane
{
	Yate &yate;
	std::vector<Pane*> panes;
	// Store Focused Pane
	Pane *focused_pane;
public:
	PaneSet(Yate &yate, Pane *parent, int x, int y, int width, int height) : Pane(parent, x, y, width, height), yate(yate) {}

	// TODO(anyone): Created better interface for proper splitting.
	void addPane(Pane *pane);
	void draw() {
		Logging::breadcrumb("Paneset Draw");
		for (auto pane : panes) {
			pane->draw();
		}
	}
	const std::vector<Pane*>& getPanes() { return panes; }
	const std::string& getTitle() { return focused_pane->getTitle(); }
	~PaneSet();
};

#endif
