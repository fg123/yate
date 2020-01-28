#include "config.h"
#include "cpptoml.h"

#include "generic-theme.h"

static Theme *theme = nullptr;
std::unordered_map<IndentationStyle, std::string, EnumClassHash>
  IndentationStyleString = {
    { IndentationStyle::TAB, "TAB" },
    { IndentationStyle::SPACE, "SPACE" }
};

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
  #undef DEFINE_TYPE
  #undef DEFINE_ENUM
  #undef DEFINE_LIST
  #define DEFINE_TYPE(key, type, default) get##key();
  #define DEFINE_ENUM(key, default) get##key();
  #define DEFINE_LIST(key, type) get##key();
  #include "config_def.h"
}

std::ostream &operator<<(std::ostream &output,
                         IndentationStyle style) {
  output << ((style == IndentationStyle::TAB) ? "Tabs" : "Spaces");
  return output;
}
