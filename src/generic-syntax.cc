#include "generic-syntax.h"

#include "util.h"

using Component = SyntaxHighlighting::Component;

std::vector<std::string> generic_keywords = {"if", "else", "for", "while"};
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
      while (start < input.size() && std::isalnum(input[start])) {
        start++;
      }
      break;
    }
    case Component::KEYWORD: {
      for (auto keyword : generic_keywords) {
        if (startsWith(keyword, actual)) {
          return start + keyword.size();
        }
      }
      break;
    }
    case Component::PREPROCESSOR:
      break;
    case Component::NUM_LITERAL: {
      while (start < input.size() && std::isdigit(input[start])) {
        start++;
      }
      break;
    }
    case Component::STR_LITERAL: {
      if (actual[0] == '"') {
        start++;
        while (start < input.size() && input[start] != '"') start++;
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