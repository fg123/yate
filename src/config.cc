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
  initializeAll();
}

YateConfig::~YateConfig() {
  delete theme;
  theme = nullptr;
}

Theme *YateConfig::getTheme() const { return theme; }

void YateConfig::initializeAll() {
  #define DEFINE_LIST(key, type) get##key();
  #define DEFINE_ENUM(key, type, default) get##key();
  #define DEFINE_OPTION(key, type, default) get##key();
  #include "config_def.h"
  #undef DEFINE_LIST
  #undef DEFINE_ENUM
  #undef DEFINE_OPTION
}

std::ostream &operator<<(std::ostream &output,
                         YateConfig::IndentationStyle style) {
  output << ((style == YateConfig::IndentationStyle::TAB) ? "Tabs" : "Spaces");
  return output;
}
