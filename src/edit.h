#ifndef EDIT_H
#define EDIT_H

#include "buffer.h"

#include <tuple>

// This module describes the notion of edits and edit history.
// Edits are insertions or deletions.
// Edits are stored in such a way that by switching the type from one to the
// other, the operation will be reversed.

using LineCol = std::tuple<LineNumber, ColNumber>;
struct EditNode {
	EditNode() {}
	~EditNode() {
		for (auto child : next) {
			delete child;
		}
	}
	enum class Type {
		INSERTION,
		DELETION
	};
	Type type;
	LineCol start;
	std::string content;
	EditNode *prev;
	std::vector<EditNode*> next;
};

#endif
