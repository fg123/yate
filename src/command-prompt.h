#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <string>

#include "prompt-window.h"

class CommandPromptWindow : public PromptWindow<std::string> {
	const std::string title = "Enter Command:";
	std::vector<std::string> items;
public:
	CommandPromptWindow(Yate &yate) : PromptWindow(yate) {
		items.push_back("Hello");
		items.push_back("World");
		items.push_back("Hello");
		items.push_back("World");
		items.push_back("Hello");
		items.push_back("World");
	}
	const std::string& getTitle() override {
		return title;
	}

	bool match(std::string buffer, size_t index) override {
		return true;
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
