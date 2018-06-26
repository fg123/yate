#include "buffer.h"
#include "editor.h"
#include "logging.h"

#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <tuple>

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
	head_edit->type = EditNode::Type::BASE_REVISION;
	head_edit->content = "This is the base revision.";
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

bool Buffer::insertNoHistory(int character, LineNumber &line, ColNumber &col) {
	if (line < 0 || line >= size()) return false;
	if (col < 0) return false;
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
	return true;
}

int Buffer::deleteNoHistory(LineNumber &line, ColNumber &col) {
	// TOOD(felixguo): Issue with undoing empty lines
	if (line < 0 || line >= size()) return 0;
	if (col < 0) return 0;
	if (col == internal_buffer[line].length() &&
		line == internal_buffer.size() - 1) return 0;
	int deleted_char;
	if (col == internal_buffer[line].length()) {
		// Join two lines
		internal_buffer[line] += internal_buffer[line + 1];
		internal_buffer.erase(internal_buffer.begin() + line + 1,
			internal_buffer.begin() + line + 2);
		deleted_char = '\n';
	}
	else {
		deleted_char = internal_buffer.at(line).at(col);
		internal_buffer.at(line).erase(col, 1);
	}
	setHasUnsavedChanges(true);
	return deleted_char;
}

void Buffer::insertCharacter(int character, LineNumber &line, ColNumber &col) {
	LineNumber orig_l = line;
	ColNumber orig_c = col;
	if (insertNoHistory(character, line, col)) {
		create_edit_for(EditNode::Type::INSERTION, character,
			orig_l, orig_c);
	}
}

void Buffer::backspace(LineNumber &line, ColNumber &col) {
	if (line < 0 || line >= size()) return;
	if (col < 0) return;
	if (!col && !line) return;
	int deleted_char;
	LineNumber orig_l = line;
	ColNumber orig_c = col;
	if (col == 0) {
		// Join two lines
		col = internal_buffer[line - 1].length();
		internal_buffer[line - 1] += internal_buffer[line];
		internal_buffer.erase(internal_buffer.begin() + line,
			internal_buffer.begin() + line + 1);
		deleted_char = '\n';
		line -= 1;
	}
	else {
		deleted_char = internal_buffer.at(line).at(col - 1);
		internal_buffer[line].erase(col - 1, 1);
		col -= 1;
	}
	create_edit_for(EditNode::Type::DELETE_BS, deleted_char, orig_l, orig_c);
	setHasUnsavedChanges(true);
}

void Buffer::_delete(LineNumber &line, ColNumber &col) {
	LineNumber orig_l = line;
	ColNumber orig_c = col;
	int deleted_char = deleteNoHistory(line, col);
	if (deleted_char) {
		create_edit_for(EditNode::Type::DELETE_DEL, deleted_char,
		orig_l, orig_c);
	}
}

ColNumber Buffer::getLineLength(LineNumber line) {
	if (line < 0 || line >= size()) return 0;
	return internal_buffer[line].length();
}

void Buffer::create_edit_for(EditNode::Type type, int character,
	const LineNumber& line, const ColNumber& col) {
	
	// Do we need new edit boundary!?
	if (!current_edit->content.empty()) {
		if (current_edit->type != type) {
			goto new_boundary;
		}
		if (current_edit->content.length() >= 20) {
			goto new_boundary;
		}

		LineNumber cur_line = std::get<0>(current_edit->start);
		ColNumber cur_col = std::get<1>(current_edit->start);
		if (current_edit->type == EditNode::Type::INSERTION) {
			if (cur_line != line || cur_col + current_edit->content.length() != col) {
				goto new_boundary;
			}
		}
		else {
			if (cur_line != line || cur_col != col) {
				// Same deletion should not move
				goto new_boundary;
			}
		}
	}
	else {
		current_edit->start = std::make_tuple(line, col);
	}
	goto perform_edit;
new_boundary:
	create_edit_boundary(line, col);
perform_edit:
	current_edit->type = type;
	if (type == EditNode::Type::DELETE_BS) {
		// TODO(felixguo): Special case with Backspace
		//std::get<1>(current_edit->start)--;
		current_edit->content.insert(0, 1, static_cast<char>(character));
	}
	else {
		current_edit->content += static_cast<char>(character);
	}
}

void Buffer::create_edit_boundary(const LineNumber& line, const ColNumber& col) {
	EditNode *n = new EditNode();
	n->start = std::make_tuple(line, col);
	n->prev = current_edit;
	current_edit->next.push_back(n);
	current_edit = n;
}

void Buffer::apply_edit_node(EditNode* node, LineNumber& line, ColNumber& col) {
	// Ensure node location can be placed.
	line = std::get<0>(node->start);
	col = std::get<1>(node->start);
	if (line >= internal_buffer.size()) return;
	if (col >= internal_buffer.at(line).length()) return;

	// These two methods are not very optimized but they will handle newline
	// Otherwise extra algorithm / edge cases will have to be developed.
	Logging::info << "Applying edit node: " << static_cast<int>(node->type) << " " << node->content << '\n';
	if (node->type == EditNode::Type::INSERTION) {
		for (auto c : node->content) {
			insertNoHistory(c, line, col);
		}
	}
	else {
		// Delete
		for (auto c : node->content) {
			deleteNoHistory(line, col);
		}
	}
}

void Buffer::undo(LineNumber& line, ColNumber& col) {
	if (current_edit->prev) {
		// Apply opposite of current_edit
		EditNode opposite = *current_edit;
		opposite.next.clear(); // For when it gets destructed.
		opposite.type = opposite.type == EditNode::Type::INSERTION ? EditNode::Type::DELETE_BS : EditNode::Type::INSERTION;
		apply_edit_node(&opposite, line, col);
		current_edit = current_edit->prev;
	}
}

void Buffer::redo(LineNumber& line, ColNumber& col, std::vector<EditNode*>::size_type index) {
	Logging::info << "Redo " << index << " " << current_edit->next.size() << '\n';
	if (index < current_edit->next.size()) {
		apply_edit_node(current_edit->next.at(index), line, col);
		current_edit = current_edit->next.at(index);
	}
}
