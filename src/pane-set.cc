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
PaneSet::PaneSet(Yate &yate, Pane *parent, std::istream& stream) : Pane(parent, stream), yate(yate) {
	Logging::breadcrumb("Deserializing PaneSet");
	std::string token;
	while (stream >> token) {
		Logging::breadcrumb(token);
		if (token == "}") break;
		if (token == "tabset") {
			stream >> token;
			addPane(new TabSet(yate, parent, stream));
		}
		else if (token == "editor") {
			stream >> token;
			addPane(new Editor(yate, parent, stream));
		}
		else if (token == "paneset") {
			stream >> token;
			addPane(new PaneSet(yate, parent, stream));
		}
		else {
			Logging::error << "Unknown token: " << token << std::endl;
			safe_exit(1);
		}
	}
}

PaneSet::~PaneSet() {
	for (auto pane : panes) {
		delete pane;
	}
}
