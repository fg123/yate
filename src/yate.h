#ifndef YATE_H
#define YATE_H

#include <ncurses.h>

#include "config.h"

class PaneSet;
class Editor;

class Yate
{
	Config config;
	PaneSet *root;
	Editor *focused_editor;
	bool last_saw_escape = false;

	bool onCapture(int result);
public:
	Yate();
	explicit Yate(Config config);
	~Yate();
	
	void setFocus(Editor *editor);
};

#endif
