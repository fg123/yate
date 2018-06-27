#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <string>

#include "prompt-window.h"

class CommandPromptWindow : public PromptWindow<std::string> {
	const std::string title = "Enter Command:";
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

	bool match(std::string buffer, std::string item) override {
		return false; // item.prompt_buffer;
	}

	const std::string getItemString(std::string *item) override {
		return *item;
	}

	void onExecute(int index) override {
		yate.exitPrompt();
	}
};

#endif
