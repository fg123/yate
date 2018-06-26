#ifndef BUFFER_H
#define BUFFER_H

// A buffer holds a reference to a file. Editors peek into the buffer. No file
// can be opened by two buffers. A buffer can be bound to no file, but must
// have a path.

#include <string>
#include <vector>
#include <cmath>
#include <tuple>

// Edits are insertions or deletions.
// Edits are stored in such a way that by switching the type from one to the
// other, the operation will be reversed.

class Editor;
using LineNumber = std::vector<std::string>::size_type;
using ColNumber = std::string::size_type;
using LineCol = std::tuple<LineNumber, ColNumber>;

struct EditNode {
	EditNode() {}
	~EditNode() {
		for (auto child : next) {
			delete child;
		}
	}
	enum class Type {
		BASE_REVISION,
		INSERTION,
		DELETE_BS,
		DELETE_DEL
	};
	Type type;
	LineCol start;
	std::string content;
	EditNode *prev;
	std::vector<EditNode*> next;
};

class BufferWindow {
	std::vector<std::string>::iterator b;
	std::vector<std::string>::iterator e;
public:
	BufferWindow(std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) : b(begin), e(end) {}
	std::vector<std::string>::iterator begin() { return b; }
	std::vector<std::string>::iterator end() { return e; }
};

class Buffer {
	bool is_bound_to_file;
	std::string path;
	std::string unsaved_path;
	std::vector<std::string> internal_buffer;
	bool has_unsaved_changes = false;
	std::vector<Editor*> registered_editors;
	EditNode* head_edit;
	EditNode* current_edit;
	void create_edit_boundary(const LineNumber& line, const ColNumber& col);
	void apply_edit_node(EditNode* node, LineNumber& line, ColNumber& col);
	void create_edit_for(EditNode::Type type, int character, const LineNumber& line, const ColNumber& col);
	bool insertNoHistory(int character, LineNumber &line, ColNumber &col);
	int deleteNoHistory(LineNumber &line, ColNumber &col);
public:
	explicit Buffer(std::string path);
	~Buffer();
	BufferWindow getBufferWindow(LineNumber start, LineNumber end);

	const bool& hasUnsavedChanges();
	const std::string& getFileName();
	size_t size();
	size_t getLineNumberFieldWidth();
	void registerEditor(Editor* editor);
	void updateTitle();
	void setHasUnsavedChanges(bool hasUnsavedChanges);
	void insertCharacter(int character, LineNumber &line, ColNumber &col);
	void backspace(LineNumber &line, ColNumber &col);
	void _delete(LineNumber &line, ColNumber &col);
	ColNumber getLineLength(LineNumber line);
	bool writeToFile();

	void undo(LineNumber& line, ColNumber& col);
	void redo( LineNumber& line, ColNumber& col, std::vector<EditNode*>::size_type index);
};

#endif
