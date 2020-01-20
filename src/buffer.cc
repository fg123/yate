#include "buffer.h"
#include <algorithm>
#include <deque>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include "editor.h"
#include "logging.h"
#include "redo-prompt.h"
#include "syntax-highlighting.h"
#include "syntax-lookup.h"
#include "util.h"
#include "yate.h"

std::string EditNode::getTypeString() const {
  switch (type) {
    case Type::INSERTION:
      return "INSERTION";
    case Type::DELETE_BS:
    case Type::DELETE_DEL:
      return "DELETION";
    case Type::REVERT:
      return "REVERT FILE";
    case Type::BASE_REVISION:
      return "BASE REVISION";
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
    else if (c == '\t')
      result += "\\t";
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
  std::ostringstream out;
  out << "(" << revision << ") " << getTypeString() << " " << getPositionPair()
      << " " << getSerializedContent();
  return out.str();
}

Buffer::Buffer(Yate& yate) : Buffer(yate, "") {}

Buffer::Buffer(Yate& yate, std::string path)
    : yate(yate),
      path(path),
      unsaved_path(" + " + path),
      head_edit(new EditNode()),
      last_save(head_edit),
      current_edit(head_edit),
      cwd(path) {
  int i = cwd.size() - 1;
  for (; i >= 0; i--) {
    if (cwd[i] == '/') {
      break;
    }
  }
  if (i < 0) {
    cwd = '.';
  } else {
    cwd = cwd.substr(0, i);
    if (cwd.empty()) {
      cwd = '.';
    }
  }

  std::ifstream file(path);
  if (file.good()) {
    std::string line;
    while (std::getline(file, line)) {
      internal_buffer.push_back(line);
      syntax_components.emplace_back(
          line.size(), SyntaxHighlighting::Component::NO_HIGHLIGHT);
      syntax_has_multiline.push_back(false);
      prefix_trie.insertWordsFromLine(line);
    }
    if (internal_buffer.empty()) {
      internal_buffer.emplace_back();
      syntax_components.emplace_back();
      syntax_has_multiline.push_back(false);
    }
  } else {
    internal_buffer.emplace_back();
    syntax_components.emplace_back();
    syntax_has_multiline.push_back(false);
  }
  head_edit->type = EditNode::Type::BASE_REVISION;
  head_edit->content = "This is the base revision.";
  head_edit->prev = nullptr;
  head_edit->revision = 0;

  /* Highlight buffer */
  highlight();
}

Buffer::~Buffer() { delete head_edit; }

void Buffer::revert(LineNumber& line, ColNumber& col) {
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
}

void Buffer::do_revert() {
  Logging::breadcrumb("Doing revert!");
  internal_buffer.clear();
  syntax_components.clear();
  syntax_has_multiline.clear();
  std::ifstream file(path);
  if (file.good()) {
    std::string line;
    while (std::getline(file, line)) {
      internal_buffer.push_back(line);
      syntax_components.emplace_back(
          line.size(), SyntaxHighlighting::Component::NO_HIGHLIGHT);
      syntax_has_multiline.push_back(false);
    }
  } else {
    internal_buffer.emplace_back();
    syntax_components.emplace_back();
    syntax_has_multiline.push_back(false);
  }
  last_save = current_edit;
  update_unsaved_marker();
  highlight();
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
  bool should_trim = yate.config.shouldTrimTrailingWhitespace();
  if (!path.empty()) {
    std::ofstream test_file(path, std::ios::app);
    if (!test_file.good()) return false;
    test_file.close();
    std::ofstream file(path, std::ios::trunc);
    for (auto line : internal_buffer) {
      if (should_trim) {
        ColNumber end = line.find_last_not_of(" \t");
        line.erase(end + 1);
      }
      file << line << std::endl;
    }
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
  if (has_unsaved_changes != hasUnsavedChanges) {
    has_unsaved_changes = hasUnsavedChanges;
    updateTitle();
  }
}

bool Buffer::insert_no_history(int character, LineNumber& line,
                               ColNumber& col) {
  if (line < 0 || line >= size()) return false;
  if (col < 0) return false;
  if (character == '\n') {
    if (col == internal_buffer[line].length()) {
      internal_buffer.insert(internal_buffer.begin() + (line + 1), "");
      syntax_components.insert(syntax_components.begin() + (line + 1), "");
      syntax_has_multiline.insert(syntax_has_multiline.begin() + (line + 1),
                                  false);
    } else {
      internal_buffer.insert(internal_buffer.begin() + line + 1,
                             internal_buffer[line].substr(col));
      internal_buffer[line].erase(col);

      syntax_components.insert(syntax_components.begin() + (line + 1),
                               syntax_components[line].substr(col));
      syntax_components[line].erase(col);
      syntax_has_multiline.insert(syntax_has_multiline.begin() + (line + 1),
                                  false);
    }
    line++;
    col = 0;
  } else {
    // To force pick an overload
    prefix_trie.remove(getWordAt(line, col));
    internal_buffer[line].insert(internal_buffer[line].begin() + col, 1,
                                 (char)character);
    syntax_components[line].insert(syntax_components[line].begin() + col, 1,
                                   SyntaxHighlighting::Component::NO_HIGHLIGHT);
    col++;

    prefix_trie.insert(getWordAt(line, col));
    if (!isIdentifierChar(character)) {
      prefix_trie.insert(getWordAt(line, col - 1));
    }
  }
  return true;
}

void Buffer::deleteWord(LineNumber& line, ColNumber& col) {
  if (line >= internal_buffer.size()) {
    return;
  }
  if (col >= internal_buffer[line].length() + 1) {
    return;
  }
  ColNumber start = col;
  ColNumber end = col;
  while (start > 0 && isIdentifierChar(internal_buffer[line][start - 1])) {
    start -= 1;
  }
  while (end < internal_buffer[line].length() &&
         isIdentifierChar(internal_buffer[line][end])) {
    end += 1;
  }
  deleteRange(std::make_tuple(line, start), std::make_tuple(line, end - 1));
  col = start;
  update_unsaved_marker();
}

std::string Buffer::getWordAt(const LineNumber& line, const ColNumber& col,
                              bool* is_at_end_of_word) {
  if (line >= internal_buffer.size()) {
    return "";
  }
  if (col >= internal_buffer[line].length() + 1) {
    return "";
  }
  ColNumber start = col;
  ColNumber end = col;
  while (start > 0 && isIdentifierChar(internal_buffer[line][start - 1])) {
    start -= 1;
  }
  while (end < internal_buffer[line].length() &&
         isIdentifierChar(internal_buffer[line][end])) {
    end += 1;
  }
  if (is_at_end_of_word) {
    *is_at_end_of_word = end == col;
  }
  return internal_buffer[line].substr(start, end - start);
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
    std::string left = getWordAt(line, col);
    std::string right = getWordAt(line + 1, 0);

    internal_buffer[line] += internal_buffer[line + 1];
    syntax_components[line] += syntax_components[line + 1];

    internal_buffer.erase(internal_buffer.begin() + line + 1,
                          internal_buffer.begin() + line + 2);
    syntax_components.erase(syntax_components.begin() + line + 1,
                            syntax_components.begin() + line + 2);

    deleted_char = '\n';

    prefix_trie.remove(left);
    prefix_trie.remove(right);
  } else {
    deleted_char = internal_buffer.at(line).at(col);
    prefix_trie.remove(getWordAt(line, col));
    internal_buffer.at(line).erase(col, 1);
    syntax_components.at(line).erase(col, 1);
  }
  prefix_trie.insert(getWordAt(line, col));
  return deleted_char;
}

void Buffer::insertCharacter(char character, LineNumber& line, ColNumber& col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  if (insert_no_history(character, line, col)) {
    create_edit_for(EditNode::Type::INSERTION, std::string(1, character),
                    orig_l, orig_c);
    update_unsaved_marker();

    highlight(character == '\n' ? line - 1 : line, line + 1);
  }
}

void Buffer::delete_line_no_history(LineNumber line) {
  if (line >= size()) return;
  internal_buffer.erase(internal_buffer.begin() + line);
  syntax_components.erase(syntax_components.begin() + line);
  syntax_has_multiline.erase(syntax_has_multiline.begin() + line);
}

void Buffer::deleteLine(LineNumber& line) {
  create_edit_for(EditNode::Type::DELETE_DEL, getLine(line) + '\n',
    line, 0);
  delete_line_no_history(line);
  update_unsaved_marker();
}

void Buffer::append_no_history(std::string& str) {
  LineNumber l = size() - 1;
  ColNumber c = getLine(l).size();
  for (auto s : str) {
    insert_no_history(s, l, c);
  }
}

void Buffer::insertString(std::string& str, LineNumber& line, ColNumber& col) {
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
  if (col == 0) {
    // Join two lines
    col = internal_buffer[line - 1].length();
    internal_buffer[line - 1] += internal_buffer[line];
    syntax_components[line - 1] += syntax_components[line];
    internal_buffer.erase(internal_buffer.begin() + line,
                          internal_buffer.begin() + line + 1);
    syntax_components.erase(syntax_components.begin() + line,
                            syntax_components.begin() + line + 1);
    deleted_char = '\n';
    line -= 1;
  } else {
    deleted_char = internal_buffer.at(line).at(col - 1);
    internal_buffer[line].erase(col - 1, 1);
    syntax_components[line].erase(col - 1, 1);
    col -= 1;
  }
  create_edit_for(EditNode::Type::DELETE_BS, std::string(1, deleted_char), line,
                  col);
  update_unsaved_marker();

  highlight(line, line + 1);
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
    syntax_components.at(from_line).erase(from_col, to_col - from_col + 1);
    goto finish;
  }

  /* Delete lines, but we leave the first and the last line */
  internal_buffer.erase(internal_buffer.begin() + from_line + 1,
                        internal_buffer.begin() + to_line);
  syntax_components.erase(syntax_components.begin() + from_line + 1,
                          syntax_components.begin() + to_line);
  syntax_has_multiline.erase(syntax_has_multiline.begin() + from_line + 1,
                             syntax_has_multiline.begin() + to_line);
  to_line = from_line + 1;

  /* Now we handle deletions on the first and last line */
  internal_buffer.at(from_line).erase(from_col);
  internal_buffer.at(to_line).erase(0, to_col);
  internal_buffer.at(from_line) += internal_buffer.at(to_line);

  syntax_components.at(from_line).erase(from_col);
  syntax_components.at(to_line).erase(0, to_col);
  syntax_components.at(from_line) += syntax_components.at(to_line);

  internal_buffer.erase(internal_buffer.begin() + to_line);
  syntax_components.erase(syntax_components.begin() + to_line);
  syntax_has_multiline.erase(syntax_has_multiline.begin() + to_line);

finish:
  highlight(from_line, internal_buffer.size());
}

std::string Buffer::getSyntax() const { return syntax_name; }

void Buffer::setSyntax(std::string syntax) {
  syntax_name = syntax;
  highlight();
}

void Buffer::highlight(LineNumber from, LineNumber to) {
  if (syntax_name.empty()) {
    // Determine Syntax
    syntax_name = SyntaxHighlighting::determineSyntax(this);
  }
  if (syntax_name != "none") {
    Syntax* syntax = SyntaxHighlighting::lookup(syntax_name);
    SyntaxHighlighting::highlight(syntax, internal_buffer, syntax_components,
                                  syntax_has_multiline, from, to);
  }
}

void Buffer::_delete(LineNumber& line, ColNumber& col) {
  LineNumber orig_l = line;
  ColNumber orig_c = col;
  char deleted_char = delete_no_history(line, col);
  if (deleted_char) {
    create_edit_for(EditNode::Type::DELETE_DEL, std::string(1, deleted_char),
                    orig_l, orig_c);
    highlight(line, line + 1);
    update_unsaved_marker();
  }
}

ColNumber Buffer::getLineLength(LineNumber line) {
  if (line < 0 || line >= size()) return 0;
  return internal_buffer[line].length();
}

void Buffer::create_edit_for(EditNode::Type type, std::string content,
                             const LineNumber& line, const ColNumber& col) {
  isInPasteMode = false;
  // Do we need new edit boundary!?
  using namespace std::chrono;
  milliseconds current_time =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  if (!current_edit->content.empty()) {
    // Different action OR not in the correct spot
    if (current_time.count() - last_modified_time.count() < 50) {
      // Pasting mode
      isInPasteMode = true;
    }

    if (current_edit->type != type) {
      // Different action
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

    if (isInPasteMode) {
      goto perform_edit;
    }

    if (current_edit->content.length() >= 20) {
      // Over the length for a boundary
      goto new_boundary;
    }
    if (content.size() != 1) {
      // Inserting a string, always make new boundary
      goto new_boundary;
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

  using namespace std::chrono;
  last_modified_time =
      duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}

void Buffer::setRevisionLock() { revision_lock = current_edit->revision + 1; }

void Buffer::clearRevisionLock() { revision_lock = 0; }

void Buffer::create_edit_boundary(const LineNumber& line,
                                  const ColNumber& col) {
  EditNode* n = new EditNode();
  n->start = std::make_tuple(line, col);
  n->prev = current_edit;
  if (revision_lock) {
    n->revision = revision_lock;
  } else {
    n->revision = current_edit->revision + 1;
  }
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
  LineNumber start = line;
  LineNumber end = line;
  if (node->type == EditNode::Type::INSERTION) {
    for (auto c : node->content) {
      insert_no_history(c, line, col);
      start = std::min(start, line);
      end = std::max(end, line);
    }
  } else if (node->type == EditNode::Type::REVERT) {
    do_revert();
    start = 0;
    end = size();
  } else {
    // Delete
    for (auto c : node->content) {
      UNUSED(c);
      Logging::breadcrumb("Deleting");
      delete_no_history(line, col);
      start = std::min(start, line);
      end = std::max(end, line);
    }
  }
  update_unsaved_marker();
  highlight(start, end + 1);
}

void Buffer::addTag(std::string label, LineNumber& line, ColNumber& col) {
  tags[label] = current_edit;
  create_edit_boundary(line, col);
}

void Buffer::fastTravel(EditNode* location, LineNumber& line, ColNumber& col) {
  Logging::breadcrumb("Fast Travelling");
  std::deque<EditNode*> destinationParents;
  std::deque<EditNode*> currentParents;

  EditNode* curr = location;
  do {
    destinationParents.push_front(curr);
    curr = curr->prev;
  } while (curr);
  curr = current_edit;
  do {
    currentParents.push_front(curr);
    curr = curr->prev;
  } while (curr);

  // [0] should have the head-edit / shared
  if (currentParents[0] != destinationParents[0]) {
    Logging::error
        << "Fast travel error, current and destination doesn't share root!"
        << std::endl;
  }

  Logging::info << "Looking for common parent, current: "
                << currentParents.size()
                << " destination: " << destinationParents.size() << std::endl;
  size_t common_index = 0;

  // First one has to be same.
  size_t i = 1, j = 1;
  while (i < currentParents.size() && j < destinationParents.size()) {
    if (i == j && currentParents[i] == destinationParents[j]) {
      i++;
      j++;
      common_index++;
    } else {
      break;
    }
  }
  Logging::info << "Common index " << common_index << std::endl;
  for (size_t i = currentParents.size() - 1; i > common_index; i--) {
    undo_highlight(line, col);
  }
  for (size_t i = common_index + 1; i < destinationParents.size(); i++) {
    redo_from_node(line, col, destinationParents[i]);
  }
}

void Buffer::undo_highlight(LineNumber& line, ColNumber& col) {
  // Apply opposite of current_edit
  int revision = current_edit->revision;
  while (current_edit->revision == revision && current_edit->prev) {
    if (current_edit->type == EditNode::Type::REVERT) {
      internal_buffer.clear();
      syntax_components.clear();
      syntax_has_multiline.clear();
      std::istringstream pre_revert(current_edit->content);
      std::string line;
      while (std::getline(pre_revert, line)) {
        internal_buffer.push_back(line);
        syntax_components.emplace_back(
            line.size(), SyntaxHighlighting::Component::NO_HIGHLIGHT);
        syntax_has_multiline.push_back(false);
      }
      // re-highlight entire file
      highlight();
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

void Buffer::undo(LineNumber& line, ColNumber& col) {
  undo_highlight(line, col);
}

void Buffer::apply_redo_step(LineNumber& line, ColNumber& col,
                             std::vector<EditNode*>::size_type index) {
  redo_highlight(line, col, index);
}

void Buffer::redo_from_node(LineNumber& line, ColNumber& col, EditNode* node) {
  int revision = node->revision;
  do {
    current_edit = node;
    if (current_edit->type == EditNode::Type::REVERT) {
      do_revert();
    } else {
      apply_edit_node(current_edit, line, col);
    }
    if (current_edit->next.size() != 1) {
      node = nullptr;
    } else {
      node = current_edit->next[0];
    }
  } while (node && node->revision == revision);
}

void Buffer::redo_highlight(LineNumber& line, ColNumber& col,
                            std::vector<EditNode*>::size_type index) {
  if (index < current_edit->next.size()) {
    redo_from_node(line, col, current_edit->next.at(index));
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
