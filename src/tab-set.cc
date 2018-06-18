#include "tab-set.h"
#include "editor.h"
#include "pane.h"
#include "logging.h"

TabSet::TabSet() {
	PaneSet *pane_s = new PaneSet();
	// TODO(felixguo): Add global buffer cache
	pane_s->addPane(new Editor(new Buffer("default.txt")));
	tabs.emplace_back(pane_s);
	selected_tab = 0;
}

TabSet::~TabSet() {
	for (auto tab : tabs) {
		delete tab;
	}
}

void TabSet::draw() {
	Logging::info("Tabset Draw");
	tabs[selected_tab]->draw();
}
