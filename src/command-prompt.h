#ifndef COMMAND_PROMPT_H
#define COMMAND_PROMPT_H

#include <string>

#include "prompt-window.h"

class CommandPromptWindow : public PromptWindow<std::string> {
	const std::string title = "Enter Command:";
public:
	CommandPromptWindow(Yate &yate) : PromptWindow(yate) {}
	const std::string& getTitle() override {
		return title;
	}

	bool match(std::string item) override {
		return true;
	}
};

#endif
