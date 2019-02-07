#ifndef GENERIC_THEME_H
#define GENERIC_THEME_H

#include "cpptoml.h"
#include "logging.h"
#include "theme.h"

#include <ncurses.h>
#include <unordered_map>

struct Color {
  unsigned short r, g, b;
};

Color from_hex_string(std::string hex) {
  Color c = {0, 0, 0};
  sscanf(hex.c_str(), "%02hx%02hx%02hx", &c.r, &c.g, &c.b);
  /* Rebalance to 0 - 1000 */
  c.r = (short)((c.r / 255.0) * 1000);
  c.g = (short)((c.g / 255.0) * 1000);
  c.b = (short)((c.b / 255.0) * 1000);
  return c;
}

/* This takes a theme.toml file and creates a theme */
class GenericTheme : public Theme {
  std::unordered_map<SyntaxHighlighting::Component, int> default_colors = {
    { SyntaxHighlighting::Component::COMMENT, 242 },
    { SyntaxHighlighting::Component::KEYWORD, 100 },
    { SyntaxHighlighting::Component::NUM_LITERAL, 169 },
    { SyntaxHighlighting::Component::STR_LITERAL, 36 },
    { SyntaxHighlighting::Component::PREPROCESSOR, 166 },
    { SyntaxHighlighting::Component::NO_HIGHLIGHT, COLOR_WHITE },
    { SyntaxHighlighting::Component::WHITESPACE, COLOR_WHITE },
    { SyntaxHighlighting::Component::IDENTIFIER, COLOR_WHITE }
  };

 public:
  GenericTheme(std::string path) {
    std::shared_ptr<cpptoml::table> config;
    try {
      config = cpptoml::parse_file(path);
    } catch (cpptoml::parse_exception e) {
      config = cpptoml::make_table();
    }
    init_pair(0, -1, -1);
    int next_color = 8;
    for (size_t i = 0; i < SyntaxHighlighting::COMPONENT_STRING.size(); i++) {
      /* This can either be an xterm-color int, or a hex color string */
      auto as_int =
          config->get_as<int>(SyntaxHighlighting::COMPONENT_STRING[i]).value_or(
            default_colors[SyntaxHighlighting::COMPONENTS[i]]);
      auto as_string =
          config->get_as<std::string>(SyntaxHighlighting::COMPONENT_STRING[i]);
      int foreground = -1;
      if (as_string) {
        std::string hex_str = *as_string;
        if (hex_str[0] == '#') {
          Color c = from_hex_string(hex_str.substr(1));
          init_color(next_color, c.r, c.g, c.b);
          foreground = next_color;
          next_color += 1;
        }
      } else {
        // TODO(felixguo): No background for now; should support?
        foreground = as_int;
      }
      init_pair((int)SyntaxHighlighting::COMPONENTS[i], foreground,
                COLOR_BLACK);
    }
  }

  ~GenericTheme() {}

  int map(SyntaxHighlighting::Component component) override {
    return COLOR_PAIR((int)component);
  }
};
#endif
