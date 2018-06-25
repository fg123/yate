// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <fstream>
#include "logging.h"
#include "pane.h"
#include "buffer.h"
#include "yate.h"
#include "focusable.h"

class Editor: public Pane, public Focusable {
	Yate &yate;
	Buffer *buffer;
	LineNumber current_line = 0;
	ColNumber current_col = 0;
	int window_start = 0;

	// For when you move cursor past an empty line
	ColNumber phantom_col_pos = 0;
	void updateColWithPhantom();
public:
	Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width, int height) :
		Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
			buffer->registerEditor(this);
	}

	void draw() override;
	const std::string& getTitle() override;
	int capture() override;
	void onKeyPress(int key) override;
};

#endif
