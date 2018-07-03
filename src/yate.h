#ifndef YATE_H
#define YATE_H

#include <vector>
#include <string>
#include <functional>

#include "src/config.pb.h"

#include "focusable.h"
#include "logging.h"

class PaneSet;
class Buffer;

template <class T>
class PromptWindow;

class Yate {
	std::string config_path;
	Config config;
	Focusable *current_focus = nullptr;
	std::vector<Buffer*> opened_buffers;
	Focusable *previous_focus = nullptr;

	// If we cast it to Focusable, the pointer will decay and won't be able to
	//   be deleted.

	void *current_prompt = nullptr;

public:
	PaneSet *root;

	explicit Yate(std::string config_path);
	~Yate();
	bool onCapture(int result);
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
	void exitPromptThenRun(std::function<void()> function);
};

#endif
