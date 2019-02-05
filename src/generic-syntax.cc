#include "generic-syntax.h"

#include "logging.h"
#include "util.h"

using Component = SyntaxHighlighting::Component;
// clang-format off
std::vector<std::string> generic_keywords = { "alignas", "alignof", "and", "and_eq",
"asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor",
"bool", "break", "case", "catch", "char", "char16_t", "char32_t", "class", "compl",
"concept", "const", "constexpr", "const_cast", "continue", "co_await", "co_return",
"co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else",
"enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto",
"if", "import", "inline", "int", "long", "module", "mutable", "namespace", "new",
"noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
"protected", "public", "register", "reinterpret_cast", "requires", "return", "short",
"signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
"synchronized", "template", "this", "thread_local", "throw", "true", "try", "typedef",
"typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile",
"wchar_t", "while", "xor", "xor_eq", "override", "final" };
// clang-format on
ColNumber GenericSyntax::match(Component component, std::string &input,
                               ColNumber start) {
  std::string actual = input.substr(start);
  switch (component) {
    case Component::COMMENT: {
      if (startsWith("//", actual)) {
        start = input.size();
      }
      break;
    }
    case Component::IDENTIFIER: {
      while (start < input.size() &&
             (std::isalnum(input[start]) || input[start] == '_')) {
        start++;
      }
      break;
    }
    case Component::KEYWORD: {
      for (auto keyword : generic_keywords) {
        if (startsWithWordBoundary(keyword, actual)) {
          return start + keyword.size();
        }
      }
      break;
    }
    case Component::PREPROCESSOR:
      if (startsWith("#", actual)) {
        start = input.size();
      }
      break;
    case Component::NUM_LITERAL: {
      while (start < input.size() && std::isdigit(input[start])) {
        start++;
      }
      break;
    }
    case Component::STR_LITERAL: {
      if (input[start] == '"') {
        start++;
        while (start < input.size() && input[start] != '"') {
          start++;
        }
        /* Consume ending quote */
        start++;
      } else if (input[start] == '\'') {
        start++;
        while (start < input.size() && input[start] != '\'') {
          start++;
        }
        /* Consume ending quote */
        start++;
      }
      break;
    }
    case Component::WHITESPACE: {
      while (start < input.size() && std::isspace(input[start])) {
        start++;
      }
      break;
    }
    default:
    case Component::NO_HIGHLIGHT:
      break;
  }
  return start;
}