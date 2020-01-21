#include "config.h"
#include "cpptoml.h"

#include "generic-theme.h"

static Theme *theme = nullptr;

YateConfig::YateConfig(std::string path) {
  try {
    internal_config = cpptoml::parse_file(path);
    wasLoadedFromFile = true;
  } catch (cpptoml::parse_exception e) {
    Logging::error << "Error parsing config TOML!" << std::endl;
    internal_config = cpptoml::make_table();
    wasLoadedFromFile = false;
  }

  theme = new GenericTheme("themes/atlas.toml");
}

YateConfig::~YateConfig() {
  delete theme;
  theme = nullptr;
}

Theme *YateConfig::getTheme() const { return theme; }

std::ostream &operator<<(std::ostream &output,
                         YateConfig::IndentationStyle style) {
  output << ((style == YateConfig::IndentationStyle::TAB) ? "Tabs" : "Spaces");
  return output;
}
