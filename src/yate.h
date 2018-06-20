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
public:
	Yate();
	explicit Yate(Config config);
	~Yate();
	
	void setFocus(Editor *editor);
	void onCapture(int result);
};

#endif
