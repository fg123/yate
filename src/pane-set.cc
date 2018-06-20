#include "pane-set.h"
#include "tab-set.h"

void PaneSet::addPane(Pane *pane) {
	if (panes.size() == 0) {
		focused_pane = pane;
	}
	panes.emplace_back(pane);
}

PaneSet::~PaneSet() {
	for (auto pane : panes) {
		delete pane;
	}
}
