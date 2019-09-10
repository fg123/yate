#include "bash.h"

#include "logging.h"
#include "util.h"

using Component = SyntaxHighlighting::Component;
// clang-format off
std::vector<std::string> bash_keywords = { "break", "case", "continue",
"do", "done", "echo", "else", "esac", "eval", "exec", "exit", "export", "fi",
"for", "if", "read", "readonly", "return", "set", "shift", "trap", "ulimit",
"umask", "unset", "until", "wait", "while" };
// clang-format on

bool BashSyntax::isMultiline(SyntaxHighlighting::Component component) {
  return component == Component::STR_LITERAL;
}

LineCol BashSyntax::match(Component component,
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
      }
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
      for (auto keyword : bash_keywords) {
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
