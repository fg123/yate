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

class Editor: public Pane {
	Yate &yate;
	Buffer *buffer;
	// TODO(anyone): Should this be 1 indexed LOL?
	int current_line = 0;
	int current_col = 0;
	int window_start = 0;
public:
	Editor(Yate &yate, Buffer *buffer, int x, int y, int width, int height) :
		Pane(x, y, width, height), yate(yate), buffer(buffer) {
	}

	void draw() override;
	const std::string& getTitle() override;
	void insertCharacter(int character);
	void backspace();
	// Because C++ delete is keyword...
	void _delete();
	void left() {
		if (current_col != 0) current_col--;
	}
	void right() {
		if (current_col != buffer->getLineLength(current_line)) current_col++;
	}
	void up() {
		if (current_line != 0) current_line--;
		current_col = std::min(current_col, buffer->getLineLength(current_line));
	}
	void down() {
		if (current_line != buffer->size() - 1) current_line++;
		current_col = std::min(current_col, buffer->getLineLength(current_line));
	}
	void home() {
		current_col = 0;
	}
	void end() {
		current_col = buffer->getLineLength(current_line);
	}
	void save();
	void focus();
	int capture();
};

#endif
