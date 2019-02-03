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
  while (output.size() < input.size()) {
    /* Fill output up to where we need it to be */
    output.push_back("");
  }
  for (LineNumber line = from; line < to; line++) {
    output.at(line) =
        std::string(input.at(line).size(), (char)Component::NO_HIGHLIGHT);
    ColNumber col = 0;
    while (col < input.at(line).size()) {
      /* Try each parse, find longest one */
      Component longest_component = Component::NO_HIGHLIGHT;
      ColNumber result_col = col;
      for (auto component : COMPONENT_MATCH_ORDER) {
        ColNumber match_result = syntax->match(component, input.at(line), col);
        if (match_result > result_col) {
          result_col = match_result;
          longest_component = component;
        }
      }

      /* Could not parse anything else */
      if (result_col == col) {
        /* We jump to next space and ignore characters */
        while (result_col < input.at(line).size() &&
               !std::isspace(input.at(line).at(result_col))) {
          result_col++;
        }
      } else {
        Logging::info << "Matched " << (int)longest_component << std::endl;
      }
      while (col < result_col) {
        output.at(line).at(col) = (char)longest_component;
        col++;
      }
    }
  }
}