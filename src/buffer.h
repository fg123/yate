#ifndef BUFFER_H
#define BUFFER_H

// A buffer holds a reference to a file. Editors peek into the buffer. No file
// can be opened by two buffers. A buffer can be bound to no file, but must
// have a path.

#include <cmath>
#include <string>
#include <tuple>
#include <vector>

// Edits are insertions or deletions.
// Edits are stored in such a way that by switching the type from one to the
// other, the operation will be reversed.

class Yate;
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
  enum class Type { BASE_REVISION, INSERTION, DELETE_BS, DELETE_DEL, REVERT };
  Type type;
  LineCol start;
  std::string content;
  EditNode *prev;
  std::vector<EditNode *> next;

  std::string getTypeString() const;
  std::string getSerializedContent() const;
  std::string getPositionPair() const;
  std::string getDescription() const;
};

class BufferWindow {
  std::vector<std::string>::iterator b;
  std::vector<std::string>::iterator e;

 public:
  BufferWindow(std::vector<std::string>::iterator begin,
               std::vector<std::string>::iterator end)
      : b(begin), e(end) {}
  std::vector<std::string>::iterator begin() { return b; }
  std::vector<std::string>::iterator end() { return e; }
};

class Buffer {
  Yate &yate;
  bool is_bound_to_file;
  std::string path;
  std::string unsaved_path;

  /* The next two buffers should always be kept in sync */
  std::vector<std::string> internal_buffer;
  std::vector<std::string> syntax_components;

  bool has_unsaved_changes = false;
  std::vector<Editor *> registered_editors;
  EditNode *head_edit;
  EditNode *last_save;
  EditNode *current_edit;
  void create_edit_boundary(const LineNumber &line, const ColNumber &col);
  void apply_edit_node(EditNode *node, LineNumber &line, ColNumber &col);
  void create_edit_for(EditNode::Type type, int character,
                       const LineNumber &line, const ColNumber &col);
  bool insert_no_history(int character, LineNumber &line, ColNumber &col);
  int delete_no_history(LineNumber &line, ColNumber &col);
  void apply_redo_step(LineNumber &line, ColNumber &col,
                       std::vector<EditNode *>::size_type index);
  void do_revert();
  void update_unsaved_marker();
  std::string getTextInRange(LineCol from, LineCol to);

 public:
  Buffer(Yate &yate, std::string path);
  ~Buffer();
  BufferWindow getBufferWindow(LineNumber start, LineNumber end);
  BufferWindow getSyntaxBufferWindow(LineNumber start, LineNumber end);

  std::string &getLine(LineNumber line) { return internal_buffer.at(line); }

  void highlight();
  const bool hasUnsavedChanges();
  const std::string &getFileName();
  size_t size();
  std::string &getPath() { return path; }
  std::vector<Editor *> &getRegisteredEditors() { return registered_editors;  }
  size_t getLineNumberFieldWidth();
  void registerEditor(Editor *editor);
  void unregisterEditor(Editor *editor);
  void updateTitle();
  void setHasUnsavedChanges(bool hasUnsavedChanges);
  void insertCharacter(int character, LineNumber &line, ColNumber &col);
  void backspace(LineNumber &line, ColNumber &col);
  void _delete(LineNumber &line, ColNumber &col);
  void deleteRange(LineCol from, LineCol to);
  ColNumber getLineLength(LineNumber line);
  bool writeToFile();
  void revert(LineNumber &line, ColNumber &col);

  void undo(LineNumber &line, ColNumber &col);
  void redo(LineNumber &line, ColNumber &col);
};

#endif
