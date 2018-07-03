#include <ncurses.h>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "yate.h"
#include "tab-set.h"
#include "logging.h"
#include "editor.h"
#include "util.h"
#include "prompt-window.h"


Yate::Yate(std::string config_path) : config_path(config_path) {
	set_escdelay(50);
	initscr();
	raw();
	noecho();
	nonl();
	start_color();
	keypad(stdscr, true);

	Logging::breadcrumb("=== Starting Yate ===");
	std::ifstream f(config_path);
	// TODO(HERE)
	// if (!f.good()) {
	// 	Logging::info << "No configuration provided. Defaulting." << std::endl;
	// 	root = new PaneSet(*this, nullptr, 0, 0, COLS, LINES);
	// 	root->addPane(new TabSet(*this, root, 0, 0, COLS, LINES));
	// }
	// else {
	// 	std::string test, test2;
	// 	f >> test >> test2;
	// 	root = new PaneSet(*this, nullptr, f);
	// }

	refresh();
	root->draw();

	if (!current_focus) {
		// TODO(felixguo): This might not be an actual issue.
		Logging::error << "No editor was initialized!" << std::endl;
		safe_exit(1);
	}
	while (true) {
		if(onCapture(current_focus->capture())) break;
	}
}

Yate::~Yate() {
	// TODO(felixguo): We might not want to truncate every time.
	std::ofstream config_file(config_path, std::fstream::trunc);
	root->serialize(config_file);

	delete root;
	for (auto buffer : opened_buffers) {
		delete buffer;
	}
	endwin();
}

void Yate::setFocus(Focusable *focus) {
	Logging::info << "Setting current to: " << focus << std::endl;
	current_focus = focus;
}

Buffer* Yate::getBuffer(std::string path) {
	// Check if path is already opened?
	auto result = std::find_if(opened_buffers.begin(), opened_buffers.end(),
		[path](Buffer* item) { return item->getPath() == path; });
	if (result != opened_buffers.end()) {
		return *result;
	}
	Buffer* buffer = new Buffer(*this, path);
	opened_buffers.push_back(buffer);
	return buffer;
}

bool Yate::onCapture(int result) {
	current_focus->onKeyPress(result);
	return result == ctrl('q');
}

void Yate::exitPromptThenRun(std::function<void()> function) {
	exitPrompt();
	function();
}

void Yate::exitPrompt() {
	if (previous_focus) {
		// TODO(anyone): Convert this prompt model to use unique pointers
		delete current_prompt;
		Logging::info << previous_focus << std::endl;
		Logging::info << current_focus << std::endl;
		setFocus(previous_focus);
		previous_focus = nullptr;
		current_prompt = nullptr;
	}
}
