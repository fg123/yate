#include <ncurses.h>
#include <cctype>

#include "yate.h"
#include "tab-set.h"
#include "logging.h"
#include "editor.h"


#define ctrl(x) ((x) & 0x1f)

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
	if (last_saw_escape) {
		last_saw_escape = false;
		Logging::info(std::to_string(result));
		if (result == 0x7f) { // escape code
			focused_editor->_delete();
		}
	}
	else {
		if (std::isprint(result)) {
			focused_editor->insertCharacter(result);
		}
		else if (result == KEY_ENTER || result == 10 || result == 13) {
			focused_editor->insertCharacter('\n');
		}
		else if (result == KEY_BACKSPACE || result == 127) {
			focused_editor->backspace();
		}
		else if (result == KEY_DC) {
			focused_editor->_delete();
		}
		else if (result == ctrl('s')) {
			focused_editor->save();
		}
		else if (result == KEY_LEFT) {
			focused_editor->left();
		}
		else if (result == KEY_RIGHT) {
			focused_editor->right();
		}
		else if (result == KEY_UP) {
			focused_editor->up();
		}
		else if (result == KEY_DOWN) {
			focused_editor->down();
		}
		else if (result == KEY_HOME) {
			focused_editor->home();
		}
		else if (result == KEY_END) {
			focused_editor->end();
		}
		else if (result == 27) {// escape code
			last_saw_escape = true;
		}
	}
	
	if (result != 'q')
		focused_editor->capture();
}