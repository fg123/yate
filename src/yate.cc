#include <ncurses.h>
#include <cctype>

#include "yate.h"
#include "tab-set.h"
#include "logging.h"
#include "editor.h"
Yate::Yate() : Yate((Config){})
{

}

Yate::Yate(Config config) : config(config)
{
	Logging::info("=== Starting Yate ===");
	root = new PaneSet(*this);
	root->addPane(new TabSet(*this, 0, 0, COLS, LINES));
	refresh();
	root->draw();
	focused_editor->capture();
}

Yate::~Yate()
{
	delete root;
}

void Yate::setFocus(Editor *editor) {
	focused_editor = editor;
}

void Yate::onCapture(int result) {
	if (std::isprint(result)) {
		focused_editor->insertCharacter(result);
	}
	else if (result == KEY_ENTER || result == 10 || result == 13) {
		focused_editor->insertCharacter('\n');
	}
	else if (result == KEY_BACKSPACE || result == KEY_DC || result == 127) {
		focused_editor->backspace();
	}
	if (result != 'q')
		focused_editor->capture();
}