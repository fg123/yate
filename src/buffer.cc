#include "buffer.h"

#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>

Buffer::Buffer(std::string path): path(path) {
	std::ifstream file(path);
	if (file.good()) {
		std::copy(std::istream_iterator<std::string>(file),
			std::istream_iterator<std::string>(),
			std::back_inserter(internal_buffer));
	}
	else {
		internal_buffer.push_back("");
	}
}


BufferWindow Buffer::getBufferWindow(int start, int end) {
	return BufferWindow(internal_buffer.begin() + start, internal_buffer.begin() + end);
}