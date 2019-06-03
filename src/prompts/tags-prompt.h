#ifndef TAGS_PROMPT_H
#define TAGS_PROMPT_H

#include "prompt-window.h"
#include "util.h"

class TagsPromptWindow : public PromptWindow {
  const std::string title = "Tags:";

  Editor *editor;
  Buffer *buffer;
  std::map<std::string, EditNode*>& tags;
  std::vector<std::string> key_lookup;
 public:
  TagsPromptWindow(Yate &yate, Editor *editor) : PromptWindow(yate), editor(editor),
    buffer(editor->getBuffer()), tags(buffer->getTags()) {
    for (auto i : tags) {
      key_lookup.push_back(i.first);
    }
  }

  const std::string &getTitle() override { return title; }
  bool match(std::string prompt_buf, size_t index) {
    std::string displayValue = getItemString(index);
    return fuzzy_match(prompt_buf, displayValue);
  }
  const std::string getItemString(size_t index) {
    if (index < tags.size()) {
      return key_lookup[index];
    }
    else {
      return "Create Tag '" + prompt_buffer + "' (creates an edit boundary)";
    }
  }
  const size_t getListSize() {
    if (prompt_buffer.empty()) return key_lookup.size();
    return key_lookup.size() + 1;
  }
  void onExecute(size_t index) {
    if (index < tags.size()) {
      editor->fastTravel(tags[key_lookup[index]]);
    }
    else {
      editor->addTag(prompt_buffer);
    }
    yate.exitPrompt();
  }
};
#endif
