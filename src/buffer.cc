#include "buffer.h"
#include "editor.h"
#include "generic-syntax.h"
#include "logging.h"
#include "redo-prompt.h"
#include "syntax-highlighting.h"
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
    if (internal_buffer.empty()) {
      internal_buffer.push_back("");
    }
  } else {
    internal_buffer.push_back("");
  }
  head_edit->type = EditNode::Type::BASE_REVISION;
  head_edit->content = "This is the base revision.";
  head_edit->prev = nullptr;

  /* Highlight buffer */
  // TODO(felixguo): determine which language
  highlight();
}

Buffer::~Buffer() { delete head_edit; }

void Buffer::revert(LineNumber &line, ColNumber &col) {
  if (!current_edit->content.empty()) {
    create_edit_boundary(line, col);
  }
  current_edit->type = EditNode::Type::REVERT;
  std::ostringstream content;
  for (auto line : internal_buffer) {
    content << line << '\n';
  }
  current_edit->content = content.str();
  do_revert();
  highlight();
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

BufferWindow Buffer::getSyntaxBufferWindow(LineNumber start, LineNumber end) {
  start = std::max((LineNumber)0, std::min(start, syntax_components.size()));
  end = std::max((LineNumber)0, std::min(end, syntax_components.size()));
  return BufferWindow(syntax_components.begin() + start,
                      syntax_components.begin() + end);
}

BufferWindow Buffer::getBufferWindow(LineNumber start, LineNumber end) {
  start = std::max((LineNumber)0, std::min(start, internal_buffer.size()));
  end = std::max((LineNumber)0, std::min(end, internal_buffer.size()));
  return BufferWindow(internal_buffer.begin() + start,
                      internal_buffer.begin() + end);
}

bool Buffer::writeToFile(LineNumber line, ColNumber col) {
  // TODO(anyone): Might be a more efficient way to write this to file
  // without deleting and rewriting it.
  std::ofstream test_file(path, std::ios::app);
  if (!test_file.good()) return false;
  test_file.close();
  std::ofstream file(path, std::ios::trunc);
  bool should_trim = yate.config.shouldTrimTrailingWhitespace();
  for (auto line : internal_buffer) {
    if (should_trim) {
      ColNumber end = line.find_last_not_of(" \t");
      line.erase(end + 1);
    }
    file << line << std::endl;
  }
  last_save = current_edit;
  update_unsaved_marker();
  if (should_trim) {
    revert(line, col);
  }
  return true;
}

void Buffer::updateTitle() {
  for (auto editor : registered_editors) {
    Logging::breadcrumb("Buffer updateTitle");
    editor->titleUpdated();
  }
}

const bool Buffer::hasUnsavedChanges() { return has_unsaved_changes; }

const std::string& Buffer::getFileName() {
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

char Buffer::delete_no_history(LineNumber& line, ColNumber& col) {
  // TOOD(felixguo): Issue with undoing empty lines
  if (line < 0 || line >= size()) return 0;
  if (col < 0) return 0;
  if (col == internal_buffer[line].length() &&
      line == internal_buffer.size() - 1)
    return 0;
  char deleted_char;
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

void Buffer::insertCharacter(char character, LineNumber& line, ColNumber& col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  if (insert_no_history(character, line, col)) {
    create_edit_for(EditNode::Type::INSERTION, std::string(1, character),
      orig_l, orig_c);
    update_unsaved_marker();

    highlight(line, character == '\n' ? internal_buffer.size() : line + 1);
  }
}

void Buffer::insertString(std::string& str, LineNumber &line, ColNumber &col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  bool has_newline = false;
  for (auto c : str) {
    if (c == '\n') has_newline = true;
    insert_no_history(c, line, col);
  }
  create_edit_for(EditNode::Type::INSERTION, str, orig_l, orig_c);
  update_unsaved_marker();

  // Highlight to current line
  highlight(orig_l, has_newline ? internal_buffer.size() : orig_l + 1);
}

void Buffer::backspace(LineNumber& line, ColNumber& col) {
  if (line < 0 || line >= size()) return;
  if (col < 0) return;
  if (!col && !line) return;
  char deleted_char;
  ColNumber highlight_to = line + 1;
  if (col == 0) {
    // Join two lines
    col = internal_buffer[line - 1].length();
    internal_buffer[line - 1] += internal_buffer[line];
    internal_buffer.erase(internal_buffer.begin() + line,
                          internal_buffer.begin() + line + 1);
    deleted_char = '\n';
    line -= 1;
    highlight_to = size();
  } else {
    deleted_char = internal_buffer.at(line).at(col - 1);
    internal_buffer[line].erase(col - 1, 1);
    col -= 1;
  }
  create_edit_for(EditNode::Type::DELETE_BS,
    std::string(1, deleted_char), line, col);
  update_unsaved_marker();

  highlight(line, highlight_to);
}

std::string Buffer::getTextInRange(LineCol from, LineCol to) {
  if (to < from) {
    LineCol tmp = from;
    from = to;
    to = tmp;
  }
  LineNumber from_line = std::get<0>(from);
  ColNumber from_col = std::get<1>(from);
  LineNumber to_line = std::get<0>(to);
  ColNumber to_col = std::get<1>(to);
  if (from_line == to_line) {
    return internal_buffer.at(from_line).substr(from_col,
                                                to_col - from_col + 1);
  }
  std::ostringstream builder;
  builder << internal_buffer.at(from_line).substr(from_col) << "\n";
  for (LineNumber line = from_line + 1; line < to_line; line++) {
    builder << internal_buffer.at(line) << "\n";
  }
  builder << internal_buffer.at(to_line).substr(0, to_col);
  return builder.str();
}

void Buffer::deleteRange(LineCol from, LineCol to) {
  if (to < from) {
    LineCol tmp = from;
    from = to;
    to = tmp;
  }
  LineNumber from_line = std::get<0>(from);
  ColNumber from_col = std::get<1>(from);
  LineNumber to_line = std::get<0>(to);
  ColNumber to_col = std::get<1>(to);

  /* EditNode for history */
  std::string content = getTextInRange(from, to);
  if (!current_edit->content.empty()) {
    create_edit_boundary(from_line, from_col);
  }
  current_edit->type = EditNode::Type::DELETE_BS;
  current_edit->content = content;

  if (from_line == to_line) {
    /* Simple same-line delete */
    internal_buffer.at(from_line).erase(from_col, to_col - from_col + 1);
    goto finish;
  }

  /* Delete lines, but we leave the first and the last line */
  internal_buffer.erase(internal_buffer.begin() + from_line + 1,
                        internal_buffer.begin() + to_line);

  to_line = from_line + 1;

  /* Now we handle deletions on the first and last line */
  internal_buffer.at(from_line).erase(from_col);
  internal_buffer.at(to_line).erase(0, to_col);
  internal_buffer.at(from_line) += internal_buffer.at(to_line);
  internal_buffer.erase(internal_buffer.begin() + to_line);

finish:
  highlight(from_line, internal_buffer.size());
}

void Buffer::highlight(LineNumber from, LineNumber to) {
  /* TODO(felixguo): only rehighlight parts that matter */
  GenericSyntax* syntax = new GenericSyntax();
  SyntaxHighlighting::highlight(syntax, internal_buffer, syntax_components, from, to);
  delete syntax;
}

void Buffer::_delete(LineNumber& line, ColNumber& col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  char deleted_char = delete_no_history(line, col);
  if (deleted_char) {
    create_edit_for(EditNode::Type::DELETE_DEL,
      std::string(1, deleted_char), orig_l, orig_c);
    highlight(line, deleted_char == '\n' ? internal_buffer.size() : line + 1);
  }
}

ColNumber Buffer::getLineLength(LineNumber line) {
  if (line < 0 || line >= size()) return 0;
  return internal_buffer[line].length();
}

void Buffer::create_edit_for(EditNode::Type type, std::string content,
                             const LineNumber& line, const ColNumber& col) {
  // Do we need new edit boundary!?
  if (!current_edit->content.empty()) {
    if (current_edit->type != type) {
      // Different action
      goto new_boundary;
    }
    if (current_edit->content.length() >= 20) {
      // Over the length for a boundary
      goto new_boundary;
    }
    if (content.size() != 1) {
      // Inserting a string, always make new boundary
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
    current_edit->content.insert(0, content);
  } else {
    current_edit->content.append(content);
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

  // TODO(felixguo): determine which language
  highlight();
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

    // TODO(felixguo): determine which language
    highlight();
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
