#include "generic.h"

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

bool GenericSyntax::isMultiline(SyntaxHighlighting::Component component) {
  return component == Component::COMMENT || component == Component::STR_LITERAL;
}

bool GenericSyntax::matchFile(Buffer* buffer) {
  return true;
}

LineCol GenericSyntax::match(Component component,
                             std::vector<std::string> &document,
                             LineCol start) {
  std::string input = document.at(LINE(start));
  std::string actual = input.substr(COL(start));
  switch (component) {
    case Component::COMMENT: {
      if (startsWith("//", actual)) {
        COL(start) = input.size();
      } else if (startsWith("#", actual)) {
        if (actual.length() < 2 || std::isspace(actual[1])) {
          COL(start) = input.size();
        }
      } else if (startsWith("/*", actual)) {
        while (LINE(start) < document.size()) {
          while (COL(start) < document.at(LINE(start)).size()) {
            if (document.at(LINE(start)).at(COL(start)) == '*') {
              if (COL(start) + 1 < document.at(LINE(start)).size() &&
                  document.at(LINE(start)).at(COL(start) + 1) == '/') {
                COL(start) += 2;
                goto done;
              }
            }
            COL(start)++;
          }
          COL(start) = 0;
          LINE(start)++;
        }
      }
    done:
      break;
    }
    case Component::IDENTIFIER: {
      while (COL(start) < input.size() &&
             (std::isalnum(input[COL(start)]) || input[COL(start)] == '_')) {
        COL(start)++;
      }
      break;
    }
    case Component::CONSTANT: {
      // All caps
      ColNumber s = COL(start);
      while (COL(start) < input.size() &&
             (std::isupper(input[COL(start)]) || input[COL(start)] == '_' ||
             (COL(start) != s && std::isdigit(input[COL(start)])))) {
        COL(start)++;
      }
      break;
    }
    case Component::KEYWORD: {
      for (auto keyword : generic_keywords) {
        if (startsWithWordBoundary(keyword, actual)) {
          COL(start) += keyword.size();
          return start;
        }
      }
      break;
    }
    case Component::PREPROCESSOR:
      if (startsWith("#", actual)) {
        while (COL(start) < input.size() && !std::isspace(input[COL(start)])) {
          COL(start)++;
        }
      }
      break;
    case Component::NUM_LITERAL: {
      while (COL(start) < input.size() && std::isdigit(input[COL(start)])) {
        COL(start)++;
      }
      break;
    }
    case Component::STR_LITERAL: {
      if (COL(start) != 0 && std::isalpha(input[COL(start) - 1])) {
        break;
      }
      if (input[COL(start)] == '"') {
        COL(start)++;
        while (COL(start) < input.size() && input[COL(start)] != '"') {
          COL(start)++;
        }
        /* Consume ending quote */
        COL(start)++;
      } else if (input[COL(start)] == '\'') {
        COL(start)++;
        while (COL(start) < input.size() && input[COL(start)] != '\'') {
          COL(start)++;
        }
        /* Consume ending quote */
        COL(start)++;
      } else if (input[COL(start)] == '<') {
        ColNumber pendingEnd = COL(start) + 1;

        while (pendingEnd < input.size() && input[pendingEnd] != '>') {
          if (std::isspace(input[pendingEnd])) break;
          pendingEnd++;
        }

        if (input[pendingEnd] == '>') {
          COL(start) = pendingEnd + 1;
        }
      }
      break;
    }
    case Component::WHITESPACE: {
      while (COL(start) < input.size() && std::isspace(input[COL(start)])) {
        COL(start)++;
      }
      break;
    }
    default:
    case Component::NO_HIGHLIGHT:
      COL(start)++;
      break;
  }
  return start;
}
