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

	// For when you move cursor past an empty line
	int phantom_col_pos = 0;
	void updateColWithPhantom() {
		int end_col = buffer->getLineLength(current_line);
		if (current_col < phantom_col_pos) {
			current_col = phantom_col_pos;
		}
		if (current_col > end_col) {
			current_col = end_col;
		}
		else {
			phantom_col_pos = current_col;
		}
	}
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
		if (current_col != 0) {
			current_col--;
			phantom_col_pos = current_col;
		}
	}
	void right() {
		if (current_col != buffer->getLineLength(current_line)) {
			current_col++;
			phantom_col_pos = current_col;
		}
	}
	void up() {
		if (current_line != 0) {
			current_line--;
			updateColWithPhantom();
		}
	}
	void down() {
		if (current_line != buffer->size() - 1) {
			current_line++;
			updateColWithPhantom();
		}
	}
	void home() {
		if (current_col != 0) {
			current_col = 0;
			phantom_col_pos = current_col;
		}
	}
	void end() {
		int end_col = buffer->getLineLength(current_line);
		if (current_col != end_col) {
			current_col = end_col;
			phantom_col_pos = current_col;
		}
	}
	void save();
	void focus();
	int capture();
};

#endif
