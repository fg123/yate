#include "pane-set.h"
#include "tab-set.h"
#include "editor.h"
#include "tab-set.h"

void PaneSet::addPane(Pane *pane) {
	if (panes.size() == 0) {
		focused_pane = pane;
	}
	panes.emplace_back(pane);
}

PaneSet::PaneSet(Yate &yate, Pane *parent, const YateConfig_State_PaneSet& fromConfig) : Pane(parent, fromConfig.pane()), yate(yate) {
	Logging::breadcrumb("Deserializing PaneSet");
	for (auto tab : fromConfig.tabsets()) {
		addPane(new TabSet(yate, parent, tab));
	}
	for (auto editor : fromConfig.editors()) {
		addPane(new Editor(yate, parent, editor));
	}
	for (auto paneset : fromConfig.panesets()) {
		addPane(new PaneSet(yate, parent, paneset));
	}
}

PaneSet::~PaneSet() {
	for (auto pane : panes) {
		delete pane;
	}
}
