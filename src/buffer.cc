#include "buffer.h"
#include "editor.h"
#include "logging.h"
#include "redo-prompt.h"
#include "util.h"
#include "yate.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <tuple>

std::string EditNode::getTypeString() const {
  switch (type) {
    case Type::INSERTION:
      return "INSERTION";
    case Type::DELETE_BS:
    case Type::DELETE_DEL:
      return "DELETION";
    case Type::REVERT:
      return "REVERT FILE";
    default:
      return "UNKNOWN TYPE";
  }
}

std::string EditNode::getSerializedContent() const {
  std::string result;
  result.reserve(content.length());
  for (auto c : content) {
    if (c == '\n')
      result += "\\n";
    else
      result += c;
  }
  return result;
}

std::string EditNode::getPositionPair() const {
  LineNumber line = std::get<0>(start);
  ColNumber col = std::get<1>(start);
  return "(" + std::to_string(line) + "L, " + std::to_string(col) + "C)";
}

std::string EditNode::getDescription() const {
  return getTypeString() + " " + getPositionPair() + " " +
          getSerializedContent();
}

Buffer::Buffer(Yate& yate, std::string path)
    : yate(yate),
      path(path),
      unsaved_path(" + " + path),
      head_edit(new EditNode()),
      last_save(head_edit),
      current_edit(head_edit) {
  std::ifstream file(path);
  if (file.good()) {
    std::string line;
    while (std::getline(file, line)) {
      internal_buffer.push_back(line);
    }
  } else {
    internal_buffer.push_back("");
  }
  head_edit->type = EditNode::Type::BASE_REVISION;
  head_edit->content = "This is the base revision.";
  head_edit->prev = nullptr;
}

Buffer::~Buffer() { delete head_edit; }

void Buffer::revert(LineNumber &line, ColNumber &col) {
  if (!current_edit->content.empty()) {
    create_edit_boundary(line, col);
  }
  current_edit->type = EditNode::Type::REVERT;
  std::ostringstream content;
  for (auto line : internal_buffer) {
    content << line << std::endl;
  }
  current_edit->content = content.str();
  do_revert();
}

void Buffer::do_revert() {
  Logging::breadcrumb("Doing revert!");
  internal_buffer.clear();
  std::ifstream file(path);
  if (file.good()) {
    std::string line;
    while (std::getline(file, line)) {
      internal_buffer.push_back(line);
    }
  } else {
    internal_buffer.push_back("");
  }
  last_save = current_edit;
  update_unsaved_marker();
}

BufferWindow Buffer::getBufferWindow(LineNumber start, LineNumber end) {
  start = std::max((LineNumber)0, std::min(start, internal_buffer.size()));
  end = std::max((LineNumber)0, std::min(end, internal_buffer.size()));
  return BufferWindow(internal_buffer.begin() + start,
                      internal_buffer.begin() + end);
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
  last_save = current_edit;
  update_unsaved_marker();
  return true;
}

void Buffer::updateTitle() {
  for (auto editor : registered_editors) {
    Logging::breadcrumb("Buffer updateTitle");
    editor->titleUpdated();
  }
}

const bool Buffer::hasUnsavedChanges() {
  return has_unsaved_changes;
}

const std::string& Buffer::getFileName() {
  // TODO(anyone): Process it so it returns just the file
  // instead of whole path?
  if (hasUnsavedChanges()) {
    return unsaved_path;
  }
  return path;
}

size_t Buffer::size() { return internal_buffer.size(); }

size_t Buffer::getLineNumberFieldWidth() {
  return std::floor(std::log10(size())) + 1;
}

void Buffer::registerEditor(Editor* editor) {
  registered_editors.push_back(editor);
}

void Buffer::unregisterEditor(Editor* editor) {
  registered_editors.erase(
      std::remove(registered_editors.begin(), registered_editors.end(), editor),
      registered_editors.end());
}

void Buffer::setHasUnsavedChanges(bool hasUnsavedChanges) {
  has_unsaved_changes = hasUnsavedChanges;
  updateTitle();
}

bool Buffer::insert_no_history(int character, LineNumber& line,
                               ColNumber& col) {
  if (line < 0 || line >= size()) return false;
  if (col < 0) return false;
  if (character == '\n') {
    if (col == internal_buffer[line].length()) {
      internal_buffer.insert(internal_buffer.begin() + (line + 1), "");
    } else {
      internal_buffer.insert(internal_buffer.begin() + line + 1,
                             internal_buffer[line].substr(col));
      internal_buffer[line].erase(col);
    }
    line++;
    col = 0;
  } else {
    // To force pick an overload
    internal_buffer[line].insert(internal_buffer[line].begin() + col, 1,
                                 (char)character);
    col++;
  }
  update_unsaved_marker();
  return true;
}

int Buffer::delete_no_history(LineNumber& line, ColNumber& col) {
  // TOOD(felixguo): Issue with undoing empty lines
  if (line < 0 || line >= size()) return 0;
  if (col < 0) return 0;
  if (col == internal_buffer[line].length() &&
      line == internal_buffer.size() - 1)
    return 0;
  int deleted_char;
  if (col == internal_buffer[line].length()) {
    // Join two lines
    internal_buffer[line] += internal_buffer[line + 1];
    internal_buffer.erase(internal_buffer.begin() + line + 1,
                          internal_buffer.begin() + line + 2);
    deleted_char = '\n';
  } else {
    deleted_char = internal_buffer.at(line).at(col);
    internal_buffer.at(line).erase(col, 1);
  }
  update_unsaved_marker();
  return deleted_char;
}

void Buffer::insertCharacter(int character, LineNumber& line, ColNumber& col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  if (insert_no_history(character, line, col)) {
    create_edit_for(EditNode::Type::INSERTION, character, orig_l, orig_c);
  }
}

void Buffer::backspace(LineNumber& line, ColNumber& col) {
  if (line < 0 || line >= size()) return;
  if (col < 0) return;
  if (!col && !line) return;
  int deleted_char;
  if (col == 0) {
    // Join two lines
    col = internal_buffer[line - 1].length();
    internal_buffer[line - 1] += internal_buffer[line];
    internal_buffer.erase(internal_buffer.begin() + line,
                          internal_buffer.begin() + line + 1);
    deleted_char = '\n';
    line -= 1;
  } else {
    deleted_char = internal_buffer.at(line).at(col - 1);
    internal_buffer[line].erase(col - 1, 1);
    col -= 1;
  }
  create_edit_for(EditNode::Type::DELETE_BS, deleted_char, line, col);
  update_unsaved_marker();
}

void Buffer::_delete(LineNumber& line, ColNumber& col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  int deleted_char = delete_no_history(line, col);
  if (deleted_char) {
    create_edit_for(EditNode::Type::DELETE_DEL, deleted_char, orig_l, orig_c);
  }
}

ColNumber Buffer::getLineLength(LineNumber line) {
  if (line < 0 || line >= size()) return 0;
  return internal_buffer[line].length();
}

void Buffer::create_edit_for(EditNode::Type type, int character,
                             const LineNumber& line, const ColNumber& col) {
  // Do we need new edit boundary!?
  if (!current_edit->content.empty()) {
    if (current_edit->type != type) {
      goto new_boundary;
    }
    if (current_edit->content.length() >= 20) {
      goto new_boundary;
    }

    LineNumber cur_line = std::get<0>(current_edit->start);
    ColNumber cur_col = std::get<1>(current_edit->start);
    if (current_edit->type == EditNode::Type::INSERTION) {
      if (cur_line != line || cur_col + current_edit->content.length() != col) {
        goto new_boundary;
      }
    } else if (current_edit->type == EditNode::Type::DELETE_BS) {
      // col is 1 behind for backspaces
      if (cur_line != line || cur_col != col + 1) {
        goto new_boundary;
      }
    } else {
      if (cur_line != line || cur_col != col) {
        // Same deletion should not move
        goto new_boundary;
      }
    }
  } else {
    current_edit->start = std::make_tuple(line, col);
  }
  goto perform_edit;
new_boundary:
  create_edit_boundary(line, col);
perform_edit:
  current_edit->type = type;
  if (type == EditNode::Type::DELETE_BS) {
    current_edit->start = std::make_tuple(line, col);
    current_edit->content.insert(current_edit->content.begin(),
                                 static_cast<char>(character));
  } else {
    current_edit->content += static_cast<char>(character);
  }
}

void Buffer::update_unsaved_marker() {
  setHasUnsavedChanges(current_edit != last_save);
}

void Buffer::create_edit_boundary(const LineNumber& line,
                                  const ColNumber& col) {
  EditNode* n = new EditNode();
  n->start = std::make_tuple(line, col);
  n->prev = current_edit;
  current_edit->next.push_back(n);
  current_edit = n;
}

void Buffer::apply_edit_node(EditNode* node, LineNumber& line, ColNumber& col) {
  line = std::get<0>(node->start);
  col = std::get<1>(node->start);

  // These two methods are not very optimized but they will handle newline
  // Otherwise extra algorithm / edge cases will have to be developed.
  Logging::info << "Applying edit node: " << static_cast<int>(node->type) << " "
                << node->content << std::endl;
  if (node->type == EditNode::Type::INSERTION) {
    for (auto c : node->content) {
      insert_no_history(c, line, col);
    }
  } else if (node->type == EditNode::Type::REVERT) {
    do_revert();
  } else {
    // Delete
    for (auto c : node->content) {
      UNUSED(c);
      Logging::breadcrumb("Deleting");
      delete_no_history(line, col);
    }
  }
}

void Buffer::undo(LineNumber& line, ColNumber& col) {
  if (current_edit->prev) {
    // Apply opposite of current_edit
    if (current_edit->type == EditNode::Type::REVERT) {
      internal_buffer.clear();
      std::istringstream pre_revert(current_edit->content);
      std::string line;
      while (std::getline(pre_revert, line)) {
        internal_buffer.push_back(line);
      }
    } else {
      EditNode opposite = *current_edit;
      opposite.next.clear();  // For when it gets destructed.
      opposite.type = opposite.type == EditNode::Type::INSERTION
                          ? EditNode::Type::DELETE_BS
                          : EditNode::Type::INSERTION;
      apply_edit_node(&opposite, line, col);
    }
    current_edit = current_edit->prev;
    update_unsaved_marker();
  }
}

void Buffer::apply_redo_step(LineNumber& line, ColNumber& col,
                             std::vector<EditNode*>::size_type index) {
  if (index < current_edit->next.size()) {
    current_edit = current_edit->next.at(index);
    if (current_edit->type == EditNode::Type::REVERT) {
      do_revert();
    } else {
      apply_edit_node(current_edit, line, col);
    }
    update_unsaved_marker();
  }
}

void Buffer::redo(LineNumber& line, ColNumber& col) {
  if (current_edit->next.empty()) return;
  if (current_edit->next.size() == 1) {
    apply_redo_step(line, col, 0);
  } else {
    // Prompt for which redo
    yate.enterPrompt(new RedoPromptWindow(
        yate, current_edit->next, [this, &line, &col](unsigned int index) {
          apply_redo_step(line, col, index);
        }));
  }
}
