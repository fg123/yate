#ifndef GENERIC_THEME_H
#define GENERIC_THEME_H

#include "cpptoml.h"
#include "logging.h"
#include "theme.h"

#include <ncurses.h>
#include <map>

/* This takes a theme.toml file and creates a theme */
class GenericTheme : public Theme {
 public:
  GenericTheme(std::string path) {
    auto config = cpptoml::parse_file(path);
    for (size_t i = 0; i < SyntaxHighlighting::COMPONENT_STRING.size(); i++) {
      auto val = config->get_as<int>(SyntaxHighlighting::COMPONENT_STRING[i]);
      if (val) {
        // TODO(felixguo): No background for now; should support?
        init_pair((int)SyntaxHighlighting::COMPONENTS[i], *val, -1);
      } else {
        init_pair((int)SyntaxHighlighting::COMPONENTS[i], -1, -1);
      }
    }
  }
  ~GenericTheme() {}
  int map(SyntaxHighlighting::Component component) override {
    return COLOR_PAIR((int)component);
  }
};
#endif
