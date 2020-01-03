#ifndef SYNTAX_HIGHLIGHTER_H
#define SYNTAX_HIGHLIGHTER_H

#include "buffer.h"

#include <string>
#include <vector>

class Syntax;

namespace SyntaxHighlighting {
/* Each syntax defines how each of these are matched, each theme defines
 *   the color mapping of each of these components */
enum Component : char {
  NO_HIGHLIGHT = 0,
  COMMENT,
  IDENTIFIER,
  CONSTANT,
  PREPROCESSOR,
  KEYWORD,
  NUM_LITERAL,
  STR_LITERAL,
  WHITESPACE
};

const std::vector<Component> COMPONENTS = {
    Component::COMMENT,      Component::IDENTIFIER, Component::CONSTANT,
    Component::PREPROCESSOR, Component::KEYWORD,    Component::NUM_LITERAL,
    Component::STR_LITERAL,  Component::WHITESPACE};

const std::vector<std::string> COMPONENT_STRING = {
    "comment", "identifier",  "constant",    "preprocessor",
    "keyword", "num-literal", "str-literal", "whitespace"};

const std::vector<Component> COMPONENT_MATCH_ORDER = {
    Component::COMMENT,     Component::PREPROCESSOR, Component::STR_LITERAL,
    Component::NUM_LITERAL, Component::KEYWORD,      Component::CONSTANT,
    Component::IDENTIFIER,  Component::WHITESPACE,   Component::NO_HIGHLIGHT};

void highlight(Syntax *syntax, std::vector<std::string> &input,
               std::vector<std::string> &output,
               std::vector<bool> &multiline_flags, LineNumber from = 0,
               LineNumber to = 0);
};  // namespace SyntaxHighlighting

class Syntax {
 public:
  virtual LineCol match(SyntaxHighlighting::Component component,
                        std::vector<std::string> &input, LineCol start) = 0;
  virtual bool isMultiline(SyntaxHighlighting::Component component) = 0;
  virtual bool matchFile(Buffer *buffer) = 0;
};

namespace std {
template <>
struct hash<SyntaxHighlighting::Component> {
  size_t operator()(const SyntaxHighlighting::Component &t) const {
    return size_t(t);
  }  // namespace std
};
}  // namespace std
#endif
