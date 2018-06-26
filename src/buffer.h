#ifndef BUFFER_H
#define BUFFER_H

// A buffer holds a reference to a file. Editors peek into the buffer. No file
// can be opened by two buffers. A buffer can be bound to no file, but must
// have a path.

#include <string>
#include <vector>
#include <cmath>

class EditNode;
class Editor;
using LineNumber = std::vector<std::string>::size_type;
using ColNumber = std::string::size_type;

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
	void apply_edit_node(EditNode* node);
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

	void undo();
	void redo(std::vector<EditNode*>::size_type index);
};

#endif
