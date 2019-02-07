#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "cpptoml.h"
#include "theme.h"

class YateConfig {
  std::shared_ptr<cpptoml::table> internal_config;

 public:
  enum class IndentationStyle { TAB, SPACE };
  explicit YateConfig(std::string path);
  ~YateConfig();
  int getTabSize() const;
  IndentationStyle getIndentationStyle() const;
  bool shouldTrimTrailingWhitespace() const;
  Theme *getTheme() const;
};

std::ostream &operator<<(std::ostream &output,
                         YateConfig::IndentationStyle style);
#endif
