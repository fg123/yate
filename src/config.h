#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "cpptoml.h"

class YateConfig {
    std::shared_ptr<cpptoml::table> internal_config;
public:
    enum class IndentationStyle {
        TAB,
        SPACE
    };
    explicit YateConfig(std::string path);
    int getTabSize() const;
    IndentationStyle getIndentationStyle() const;
};

std::ostream &operator<<(std::ostream &output, YateConfig::IndentationStyle style);
#endif