#ifndef REDO_PROMPT_H
#define REDO_PROMPT_H

#include <functional>

#include "prompt-window.h"
#include "buffer.h"

class RedoPromptWindow : public PromptWindow<EditNode*> {
	const std::string title = "Select a Redo:";
	std::vector<EditNode*>& items;
	std::function<void(int)> callback;
public:
	RedoPromptWindow(Yate &yate, std::vector<EditNode*>& items,
		std::function<void(int)> callback) :
		PromptWindow(yate), items(items), callback(callback) {
		input_enabled = false;
	}

	const std::string& getTitle() override {
		return title;
	}

	bool match(std::string buffer, size_t index) override {
		return true; // No matching
	}

	const std::string getItemString(size_t index) override {
		return std::to_string(index + 1) + ": " + items.at(index)->getDescription();
	}

	void onExecute(size_t index) override {
		callback(index);
		yate.exitPrompt();
	}

	const std::vector<EditNode*>& getItems() {
		return items;
	}
};

#endif
