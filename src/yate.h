#ifndef YATE_H
#define YATE_H

#include <ncurses.h>

#include "config.h"
#include "pane-set.h"

class Yate
{
	Config config;
	PaneSet *root;
	Editor *focused_editor;
public:
	Yate();
	explicit Yate(Config config);
	~Yate();
};

#endif
