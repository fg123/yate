#include <ncurses.h>
#include <cctype>

#include "yate.h"
#include "tab-set.h"
#include "logging.h"
#include "editor.h"
#include "util.h"
#include "prompt-window.h"

Yate::Yate() : Yate((Config){}) {

}

Yate::Yate(Config config) : config(config) {
	Logging::breadcrumb("=== Starting Yate ===");
	root = new PaneSet(*this, nullptr, 0, 0, COLS, LINES);
	root->addPane(new TabSet(*this, root, 0, 0, COLS, LINES));
	refresh();
	root->draw();
	while (true) {
		if(onCapture(current_focus->capture())) break;
	}
}

Yate::~Yate() {
	delete root;
	for (auto buffer : opened_buffers) {
		delete buffer;
	}
}

void Yate::setFocus(Focusable *focus) {
	Logging::info << "Setting current to: " << focus << "\n";
	current_focus = focus;
}

Buffer* Yate::getBuffer(std::string path) {
	// Check if path is already opened?
	Buffer* buffer = new Buffer(path);
	opened_buffers.push_back(buffer);
	return buffer;
}

bool Yate::onCapture(int result) {
	current_focus->onKeyPress(result);
	return result == ctrl('q');
}

void Yate::exitPrompt() {
	if (previous_focus) {
		// TODO(anyone): Convert this prompt model to use unique pointers
		delete current_prompt;
		Logging::info << previous_focus << "\n";
		Logging::info << current_focus << "\n";
		setFocus(previous_focus);
		previous_focus = nullptr;
		current_prompt = nullptr;
	}
}
