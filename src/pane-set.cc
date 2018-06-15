#include "pane-set.h"
#include "tab-set.h"

PaneSet::PaneSet()
{

}

void PaneSet::addPane(Pane *pane)
{
	panes.emplace_back(pane);
}

PaneSet::~PaneSet()
{
	for (auto pane : panes) {
		delete pane;
	}
}
