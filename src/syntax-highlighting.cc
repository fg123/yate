#include "syntax-highlighting.h"
#include "logging.h"

void SyntaxHighlighting::highlight(Syntax *syntax,
                                   std::vector<std::string> &input,
                                   std::vector<std::string> &output,
                                   LineNumber from, LineNumber to) {
  if (to == 0 || to > input.size()) {
    /* Highlight all if default */
    to = input.size();
  }
  if (from > to) {
    /* swap */
    std::swap(from, to);
  }
  if (from == to) {
    /* nothing to highlight */
    return;
  }
  Logging::info << "Highlighting from " << from << " to " << to << std::endl;
  for (LineNumber line = from; line < to; line++) {
    std::string empty =
        std::string(input.at(line).size(), (char)Component::NO_HIGHLIGHT);
    if (output.size() <= line) {
      output.push_back(empty);
    } else {
      output[line] = empty;
    }
  }
  LineCol start = std::make_tuple(from, 0);
  while (LINE(start) < to) {
    /* Try each parse, find longest one */
    Component longest_component = Component::NO_HIGHLIGHT;
    LineCol result = start;
    for (auto component : COMPONENT_MATCH_ORDER) {
      LineCol match_result = syntax->match(component, input, start);
      if (match_result > result) {
        result = match_result;
        longest_component = component;
      }
    }
    Logging::info << "Longest " << LINE(result) << " " << COL(result) << " as "
                  << (int)longest_component << std::endl;
    if (result == start) {
      /* Could not parse anything! Move pointer to next non-whitespace. */
      while (LINE(result) < input.size() &&
             COL(result) < input.at(LINE(result)).size() &&
             !std::isspace(input.at(LINE(result)).at(COL(result)))) {
        COL(result)++;
        if (COL(result) >= input.at(LINE(result)).size()) {
          LINE(result)++;
          COL(result) = 0;
        }
      }
    }
    for (LineNumber l = LINE(start); l <= LINE(result); l++) {
      ColNumber end = std::min(COL(result), input.at(l).size());
      for (ColNumber c = COL(start); c < end; c++) {
        output.at(l).at(c) = (char)longest_component;
      }
    }
    if (COL(result) >= input.at(LINE(result)).size()) {
      LINE(result)++;
      COL(result) = 0;
    }
    start = result;
  }
}
