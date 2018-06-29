#ifndef YATE_H
#define YATE_H

#include <ncurses.h>
#include <vector>
#include <string>

#include "config.h"
#include "focusable.h"
#include "logging.h"

class PaneSet;
class Buffer;

template <class T>
class PromptWindow;

class Yate {
	Config config;
	Focusable *current_focus = nullptr;
	std::vector<Buffer*> opened_buffers;
	bool onCapture(int result);
	Focusable *previous_focus = nullptr;

	// If we cast it to Focusable, the pointer will decay and won't be able to
	//   be deleted.

	void *current_prompt = nullptr;

public:
	PaneSet *root;

	explicit Yate(Config config);
	~Yate();
	Buffer* getBuffer(std::string path);
	void setFocus(Focusable *editor);
	bool hasFocus() { return current_focus; }
	template <class T>
	void enterPrompt(PromptWindow<T> *window) {
		if (previous_focus) {
			delete window;
		}
		else {
			previous_focus = current_focus;
			Logging::info << "Focus into: " << window << std::endl;
			Logging::info << "Current: " << current_focus << std::endl;
			setFocus(window);
			current_prompt = window;
		}
	}

	void exitPrompt();
};

#endif
