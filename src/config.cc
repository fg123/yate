#include "config.h"
#include "cpptoml.h"

#include "generic-theme.h"

static int tab_size;
static YateConfig::IndentationStyle indentation_style;
static Theme *theme = nullptr;

YateConfig::YateConfig(std::string path) {
  try {
    internal_config = cpptoml::parse_file(path);
  } catch (cpptoml::parse_exception e) {
    Logging::error << "Error parsing config TOML!" << std::endl;
    internal_config = cpptoml::make_table();
  }

  tab_size = internal_config->get_as<int>("tab_size").value_or(4);
  indentation_style = (YateConfig::IndentationStyle)internal_config
                          ->get_as<int>("indentation_style")
                          .value_or(0);
  theme = new GenericTheme("themes/solarized.toml");
}

YateConfig::~YateConfig() {
  delete theme;
  theme = nullptr;
}

int YateConfig::getTabSize() const { return tab_size; }

YateConfig::IndentationStyle YateConfig::getIndentationStyle() const {
  return indentation_style;
}

Theme *YateConfig::getTheme() const { return theme; }
std::ostream &operator<<(std::ostream &output,
                         YateConfig::IndentationStyle style) {
  output << ((style == YateConfig::IndentationStyle::TAB) ? "Tabs" : "Spaces");
  return output;
}