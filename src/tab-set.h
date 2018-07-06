// Represents a tabbed set.
#ifndef TAB_SET_H
#define TAB_SET_H

#include <vector>
#include <ncurses.h>
#include <iostream>

#include "pane-set.h"
#include "pane.h"
#include "util.h"

#include "src/config.pb.h"

class TabSet : public Pane
{
	Yate &yate;
	std::vector<PaneSet *> tabs;
	unsigned int selected_tab = 0;
	void drawTabs();

  public:
	TabSet(Yate &yate, Pane *parent, int x, int y, int width, int height);
	~TabSet();
	void draw() override;
	const std::string &getTitle() override;
	void onTitleUpdated() override;
	std::ostream &serialize(std::ostream &stream) override
	{
		stream << "tabset {" << std::endl;
		stream << x << " " << y << " " << width << " " << height << std::endl;
		for (auto paneSet : tabs)
		{
			paneSet->serialize(stream);
		}
		stream << "}" << std::endl;
		return stream;
	}

	TabSet(Yate &yate, Pane *parent, const YateConfig_State_TabSet &fromConfig) : Pane(parent, fromConfig.pane()), yate(yate)
	{
		Logging::breadcrumb("Deserializing TabSet");
		for (auto paneset : fromConfig.panesets())
		{
			tabs.push_back(new PaneSet(yate, parent, paneset));
		}
	}
};

#endif
