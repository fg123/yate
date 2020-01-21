#include "syntax-highlighting.h"
#include "logging.h"

void SyntaxHighlighting::highlight(Syntax *syntax,
                                   std::vector<std::string> &input,
                                   std::vector<std::string> &output,
                                   std::vector<bool> &multiline_flags,
                                   LineNumber from, LineNumber to) {
  if (to == 0) {
    // Default parameter
    to = input.size();
  }
  if (from > to) {
    /* swap */
    std::swap(from, to);
  }
  if (to > input.size()) {
    /* Highlight all if default */
    to = input.size();
  }
  if (from < 0) {
    from = 0;
  }
  if (from == to) {
    /* nothing to highlight */
    return;
  }

  for (LineNumber line = from; line < to; line++) {
    std::string empty =
        std::string(input.at(line).size(), (char)Component::NO_HIGHLIGHT);
    multiline_flags[line] = false;
    output[line] = empty;
  }
  if (from > 0) {
    if (multiline_flags.at(from - 1)) {
      while (from > 0 && multiline_flags[from - 1]) {
        from -= 1;
      }
    }
  }
  if (to < input.size() - 1) {
    if (multiline_flags.at(to + 1)) {
      while (to < input.size() - 1 && multiline_flags[to + 1]) {
        to += 1;
      }
    }
  }
  Logging::info << "Highlighting from " << from << " to " << to << std::endl;
  LineCol start = LINECOL(from, 0);
  while (LINE(start) < to) {
    /* Try each parse, find longest one */
    Component longest_component = Component::NO_HIGHLIGHT;
    LineCol result = start;
    for (auto &component : COMPONENT_MATCH_ORDER) {
      LineCol match_result = syntax->match(component, input, start);
      if (match_result > result) {
        result = match_result;
        longest_component = component;
      }
    }
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
    if (LINE(result) >= input.size()) {
      // Too far, move it back
      LINE(result) = input.size() - 1;
      COL(result) = input.at(LINE(result)).size();
    }
    for (LineNumber l = LINE(start); l <= LINE(result); l++) {
      ColNumber end = l == LINE(result)
                          ? std::min(COL(result), input.at(l).size())
                          : input.at(l).size();
      if (syntax->isMultiline(longest_component)) {
        multiline_flags.at(l) = true;
      }
      ColNumber col_start = l == LINE(start) ? COL(start) : 0;
      for (ColNumber c = col_start; c < end; c++) {
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
