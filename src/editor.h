// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include <vector>
#include <fstream>
#include "logging.h"
#include "pane.h"
#include "buffer.h"

class Editor: public Pane {
	Buffer *buffer;
	unsigned int line;
	// TODO(anyone): Should this be 1 indexed LOL?
	int current_line = 0;
	int window_start = 0;
public:
	Editor(Buffer *buffer, int x, int y, int width, int height) :
		Pane(x, y, width, height), buffer(buffer) {
	}

	void draw() override;
	void focus();
};

#endif
