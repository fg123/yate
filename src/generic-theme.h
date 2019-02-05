#ifndef GENERIC_THEME_H
#define GENERIC_THEME_H

#include "cpptoml.h"
#include "logging.h"
#include "theme.h"

#include <ncurses.h>
#include <map>

struct Color {
  short r, g, b;
};

Color from_hex_string(std::string hex) {
  Color c = {0, 0, 0};
  sscanf(hex.c_str(), "%02x%02x%02x", &c.r, &c.g, &c.b);
  /* Rebalance to 0 - 1000 */
  c.r = (short)((c.r / 255.0) * 1000);
  c.g = (short)((c.g / 255.0) * 1000);
  c.b = (short)((c.b / 255.0) * 1000);
  return c;
}

/* This takes a theme.toml file and creates a theme */
class GenericTheme : public Theme {
 public:
  GenericTheme(std::string path) {
    auto config = cpptoml::parse_file(path);
    init_pair(0, -1, -1);
    int next_color = 8;
    for (size_t i = 0; i < SyntaxHighlighting::COMPONENT_STRING.size(); i++) {
      /* This can either be an xterm-color int, or a hex color string */
      auto as_int =
          config->get_as<int>(SyntaxHighlighting::COMPONENT_STRING[i]);
      auto as_string =
          config->get_as<std::string>(SyntaxHighlighting::COMPONENT_STRING[i]);
      int foreground = -1;
      if (as_int) {
        // TODO(felixguo): No background for now; should support?
        foreground = *as_int;
      } else if (as_string) {
        std::string hex_str = *as_string;
        if (hex_str[0] == '#') {
          Color c = from_hex_string(hex_str.substr(1));
          init_color(next_color, c.r, c.g, c.b);
          foreground = next_color;
          next_color += 1;
        }
      } else {
        foreground = COLOR_WHITE;
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
