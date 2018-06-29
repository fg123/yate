// Represents a tabbed set.
#ifndef TAB_SET_H
#define TAB_SET_H

#include <vector>
#include <ncurses.h>
#include <iostream>

#include "pane-set.h"
#include "pane.h"
#include "util.h"

class TabSet: public Pane
{
	Yate &yate;
	std::vector<PaneSet*> tabs;
	unsigned int selected_tab = 0;
	void drawTabs();
public:
	TabSet(Yate &yate, Pane *parent, int x, int y, int width, int height);
	~TabSet();
	void draw() override;
	const std::string& getTitle() override;
	void onTitleUpdated() override;
		std::ostream& serialize(std::ostream& stream) override {
		stream << "tabset {" << std::endl;
		stream << x << " " << y << " " << width << " " << height << std::endl;
		for (auto paneSet : tabs) {
			paneSet->serialize(stream);
		}
		stream << "}" << std::endl;
		return stream;
	}

	TabSet(Yate &yate, Pane *parent, std::istream& stream) : Pane(parent, stream), yate(yate) {
		std::string token;
		Logging::breadcrumb("Deserializing TabSet");
		while (stream >> token) {
			Logging::breadcrumb(token);
			if (token == "}") break;
			if (token == "paneset") {
				stream >> token;
				tabs.push_back(new PaneSet(yate, parent, stream));
				selected_tab = 0;
			}
			else {
				Logging::error << "Unknown token: " << token << std::endl;
				safe_exit(1);
			}
		}
	}
};

#endif
