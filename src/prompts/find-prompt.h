#ifndef FIND_PROMPT_H
#define FIND_PROMPT_H

#include "prompt-window.h"
#include "util.h"

class FindPromptWindow : public PromptWindow {
  const std::string title = "Find in file:";

  Editor *editor;
  Buffer *buffer;

 public:
  enum class Direction { Previous, Next };

  FindPromptWindow(Yate &yate, Editor *editor)
      : PromptWindow(yate), editor(editor), buffer(editor->getBuffer()) {}
  FindPromptWindow(Yate &yate, Editor *editor, const std::string &initial)
      : FindPromptWindow(yate, editor) {
    prompt_buffer = initial;
  }

  FindPromptWindow(Yate &yate, Editor *editor, Direction dir)
      : FindPromptWindow(yate, editor, editor->lastSearch) {
    if (dir == Direction::Previous) {
      highlighted_index = editor->lastSearchIndex - 1;
    } else {
      highlighted_index = editor->lastSearchIndex + 1;
    }

    yate.queueNextTick([&]() {
      std::vector<size_t> matched_items = get_matching_items();
      limitHighlightedIndex(matched_items);
      if (highlighted_index >= 0 &&
          (size_t)highlighted_index < matched_items.size()) {
        onExecute(matched_items.at(highlighted_index));
      } else {
        yate.exitPrompt();
      }
    });
  }

  const std::string &getTitle() override { return title; }
  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, buffer->getLine(index));
  }

  const std::string getItemString(size_t index) {
    if (prompt_buffer.empty()) return "";
    int width = buffer->getLineNumberFieldWidth();
    std::string istring = std::to_string(index);

    return std::string(width - istring.size(), ' ') + istring + ": " +
           buffer->getLine(index);
  }

  const std::string getSyntaxHighlight(size_t index) {
    if (prompt_buffer.empty()) return "";
    int width = buffer->getLineNumberFieldWidth() + 1;
    return std::string(width + 1, '\0') + buffer->getSyntax(index);
  }

  const size_t getListSize() {
    if (prompt_buffer.empty()) return 0;
    return buffer->size();
  }

  void onExecute(size_t index) {
    editor->lastSearch = prompt_buffer;
    editor->lastSearchIndex = highlighted_index;
    ColNumber col;
    fuzzy_match(prompt_buffer, buffer->getLine(index), col);
    editor->goToLineCol(index, col);
    yate.exitPrompt();
  }
};
#endif
