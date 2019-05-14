#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "cpptoml.h"
#include "theme.h"

class YateConfig {
  std::shared_ptr<cpptoml::table> internal_config;

 public:
  bool wasLoadedFromFile = false;

  enum class IndentationStyle { TAB, SPACE };
  explicit YateConfig(std::string path);
  ~YateConfig();
  int getTabSize() const;
  void setTabSize(int size);

  IndentationStyle getIndentationStyle() const;
  void setIndentationStyle(IndentationStyle style);

  bool shouldTrimTrailingWhitespace() const;
  Theme *getTheme() const;

};

std::ostream &operator<<(std::ostream &output,
                         YateConfig::IndentationStyle style);
#endif
