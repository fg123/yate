#ifndef BUFFER_H
#define BUFFER_H

// A buffer holds a reference to a file. Editors peek into the buffer. No file
// can be opened by two buffers. A buffer can be bound to no file, but must
// have a path.

#include <string>
#include <vector>
#include <cmath>

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
	std::vector<std::string> internal_buffer;
	bool has_unsaved_changes = false;
public:
	explicit Buffer(std::string path);
	BufferWindow getBufferWindow(int start, int end);
	
	const bool& hasUnsavedChanges() { return has_unsaved_changes; }
	const std::string& getFileName() {
		// TODO(anyone): Process it so it returns just the file
		// instead of whole path?
		return path;
	}

	size_t size() { return internal_buffer.size(); }
	
	size_t getLineNumberFieldWidth() {
		return std::floor(std::log10(size())) + 1;
	}

	void insertCharacter(int character, int &line, int &col) {
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
		has_unsaved_changes = true;
	}

	void backspace(int &line, int &col) {
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
		has_unsaved_changes = true;
	}

	void _delete(int &line, int &col) {
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
		has_unsaved_changes = true;
	}

	int getLineLength(int line) {
		if (line < 0 || line >= size()) return 0;
		return internal_buffer[line].length();
	}

	bool writeToFile();
};
#endif
