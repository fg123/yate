#include "buffer.h"
#include "editor.h"
#include "logging.h"

#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>

Buffer::Buffer(std::string path): path(path), unsaved_path(" + " + path) {
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
