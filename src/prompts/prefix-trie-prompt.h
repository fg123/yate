#ifndef PREFIX_TRIE_PROMPT_H
#define PREFIX_TRIE_PROMPT_H

#include "prompt-window.h"
#include "util.h"

class PrefixTrieWindow : public PromptWindow {
  Editor *editor;
  Buffer *buffer;
  const std::string title;

  std::vector<std::string> entries;
 public:
  PrefixTrieWindow(Yate &yate, Editor *editor) : PromptWindow(yate),
      editor(editor), buffer(editor->getBuffer()), title("Prefix Trie Entries") {
    entries = buffer->prefix_trie.getAllEntries();
    for (std::string& r : entries) {
      r.erase(0, 1);
    }
  }

  const std::string &getTitle() override { return title; }

  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, entries[index]);
  }

  const std::string getItemString(size_t index) {
    return entries[index];
  }

  const size_t getListSize() {
    return entries.size();
  }

  void onExecute(size_t index) {
    yate.exitPrompt();
  }
};
#endif
