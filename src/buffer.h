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
public:
	explicit Buffer(std::string path);
	BufferWindow getBufferWindow(int start, int end);
	size_t size() { return internal_buffer.size(); }
	size_t getLineNumberFieldWidth() {
		return std::floor(std::log10(size())) + 1;
	}
};
#endif
