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
	void init() {
		buffer->registerEditor(this);
		if (!yate.hasFocus()) {
			Logging::info << "Yate has no focused; setting focus." << std::endl;
			yate.setFocus(this);
		}
	}
public:
	Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width, int height) :
		Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
		init();
	}

	void draw() override;
	const std::string& getTitle() override;
	int capture() override;
	void onKeyPress(int key) override;

	std::ostream& serialize(std::ostream& stream) override {
		stream << "editor {" << std::endl;
		stream << x << " " << y << " " << width << " " << height << std::endl;
		stream << buffer->path << std::endl;
		stream << "}" << std::endl;
		return stream;
	}

	Editor(Yate &yate, Pane *parent, std::istream& stream) : Pane(parent, stream), yate(yate) {
		Logging::breadcrumb("Deserializing Editor");
		std::string token;
		std::string path;
		stream >> path >> token;
		buffer = yate.getBuffer(path);
		init();
	}
};

#endif
