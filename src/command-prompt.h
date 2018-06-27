#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <string>

#include "prompt-window.h"

class CommandPromptWindow : public PromptWindow<std::string> {
	const std::string title = "Enter Command:";
	std::vector<std::string> items;
public:
	CommandPromptWindow(Yate &yate) : PromptWindow(yate) {
		items.push_back("Using GNU Readline; how can I add ncurses in the same program?");
		items.push_back("Writing a command line shell with C; trying to use ncurses/C for the first time");
		items.push_back("Writing a console-based C++ IRC-client");
		items.push_back("Working with curses in IPython. How can I improve this?");
		items.push_back("Haskell `ncurses` library");
		items.push_back("In ncurses, is there a simple way to use every combination of the 8 standard foreground and background colors?");
		items.push_back("Cross compiled ncurses for UTF-8 displays incorrectly");
		items.push_back("Interrupt (n)curses getch on incoming signal");
		items.push_back("Extracting wide chars w/ attributes in ncurses");
		items.push_back("Porting an old DOS TUI to ncurses");
	}
	const std::string& getTitle() override {
		return title;
	}

	bool match(std::string buffer, size_t index) override {
		return std::search(
			items.at(index).begin(), items.at(index).end(),
			buffer.begin(), buffer.end(),
			[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
		) != items.at(index).end();
	}

	const std::string getItemString(size_t index) override {
		return items[index];
	}

	void onExecute(size_t index) override {
		yate.exitPrompt();
	}

	const std::vector<std::string>& getItems() {
		return items;
	}
};

#endif
