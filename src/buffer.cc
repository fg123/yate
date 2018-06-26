#include "buffer.h"
#include "editor.h"
#include "logging.h"
#include "edit.h"

#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>

Buffer::Buffer(std::string path): path(path), unsaved_path(" + " + path),
	head_edit(new EditNode()), current_edit(head_edit) {
	std::ifstream file(path);
	if (file.good()) {
		std::string line;
		while (std::getline(file, line)) {
			internal_buffer.push_back(line);
		}
	}
	else {
		internal_buffer.push_back("");
	}
}

Buffer::~Buffer() {
	delete head_edit;
}

BufferWindow Buffer::getBufferWindow(LineNumber start, LineNumber end) {
	start = std::max((LineNumber) 0, std::min(start, internal_buffer.size()));
	end = std::max((LineNumber) 0, std::min(end, internal_buffer.size()));
	return BufferWindow(internal_buffer.begin() + start, internal_buffer.begin() + end);
}

bool Buffer::writeToFile() {
	// TODO(anyone): Might be a more efficient way to write this to file
	// without deleting and rewriting it.
	std::ofstream test_file(path, std::ios::app);
	if (!test_file.good()) return false;
	test_file.close();
	std::ofstream file(path, std::ios::trunc);
	for (auto line : internal_buffer) {
		file << line << std::endl;
	}
	setHasUnsavedChanges(false);
	return true;
}

void Buffer::updateTitle() {
	for (auto editor : registered_editors) {
		Logging::breadcrumb("Buffer updateTitle");
		editor->titleUpdated();
	}
}

const bool& Buffer::hasUnsavedChanges() { return has_unsaved_changes; }

const std::string& Buffer::getFileName() {
	// TODO(anyone): Process it so it returns just the file
	// instead of whole path?
	if (hasUnsavedChanges()) {
		return unsaved_path;
	}
	return path;
}

size_t Buffer::size() { return internal_buffer.size(); }

size_t Buffer::getLineNumberFieldWidth() {
	return std::floor(std::log10(size())) + 1;
}

void Buffer::registerEditor(Editor* editor) {
	registered_editors.push_back(editor);
}


void Buffer::setHasUnsavedChanges(bool hasUnsavedChanges) {
	has_unsaved_changes = hasUnsavedChanges;
	updateTitle();
}

void Buffer::insertCharacter(int character, LineNumber &line, ColNumber &col) {
	if (line < 0 || line >= size()) return;
	if (col < 0) return;
	if (character == '\n') {
		if (col == internal_buffer[line].length()) {
			internal_buffer.insert(internal_buffer.begin() + (line + 1), "");
		}
		else {
			internal_buffer.insert(internal_buffer.begin() + line + 1, internal_buffer[line].substr(col));
			internal_buffer[line].erase(col);
		}
		line++;
		col = 0;
	}
	else {
		// To force pick an overload
		internal_buffer[line].insert(internal_buffer[line].begin() + col, 1, (char)character);
		col++;
	}
	setHasUnsavedChanges(true);
}

void Buffer::backspace(LineNumber &line, ColNumber &col) {
	if (line < 0 || line >= size()) return;
	if (col < 0) return;
	if (!col && !line) return;
	if (col == 0) {
		// Join two lines
		col = internal_buffer[line - 1].length();
		internal_buffer[line - 1] += internal_buffer[line];
		internal_buffer.erase(internal_buffer.begin() + line,
			internal_buffer.begin() + line + 1);
		line -= 1;
	}
	else {
		internal_buffer[line].erase(col - 1, 1);
		col -= 1;
	}
	setHasUnsavedChanges(true);
}

void Buffer::_delete(LineNumber &line, ColNumber &col) {
	if (line < 0 || line >= size()) return;
	if (col < 0) return;
	if (col == internal_buffer[line].length() &&
		line == internal_buffer.size() - 1) return;
	if (col == internal_buffer[line].length()) {
		// Join two lines
		internal_buffer[line] += internal_buffer[line + 1];
		internal_buffer.erase(internal_buffer.begin() + line + 1,
			internal_buffer.begin() + line + 2);
	}
	else {
		internal_buffer[line].erase(col, 1);
	}
	setHasUnsavedChanges(true);
}

ColNumber Buffer::getLineLength(LineNumber line) {
	if (line < 0 || line >= size()) return 0;
	return internal_buffer[line].length();
}

void Buffer::create_edit_boundary(const LineNumber& line, const ColNumber& col) {
	EditNode *n = new EditNode();
	n->start = std::make_tuple(line, col);
	n->prev = current_edit;
	current_edit->next.push_back(n);
	current_edit = n;
}

void Buffer::apply_edit_node(EditNode* node) {

}

void Buffer::undo() {
	if (current_edit->prev) {
		// Apply opposite of current_edit
		EditNode opposite = *current_edit;
		opposite.type = opposite.type == EditNode::Type::INSERTION ? EditNode::Type::DELETION : EditNode::Type::INSERTION;
		apply_edit_node(&opposite);
		current_edit = current_edit->prev;
	}
}

void Buffer::redo(std::vector<EditNode*>::size_type index) {
	// TODO()
}