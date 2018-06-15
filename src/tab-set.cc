#include "tab-set.h"
#include "editor.h"
#include "pane.h"

TabSet::TabSet()
{
	PaneSet *pane_s = new PaneSet();
	pane_s->addPane(new Editor("default.txt"));
	tabs.emplace_back(pane_s);
}

TabSet::~TabSet()
{
	for (auto tab : tabs) {
		delete tab;
	}
}

void TabSet::draw(WINDOW *window)
{

}
