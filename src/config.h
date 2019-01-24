#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "cpptoml.h"

class YateConfig {
    std::shared_ptr<cpptoml::table> internal_config;
public:
    enum class IndentationStyle {
        SPACE,
        TAB
    };
    explicit YateConfig(std::string path);
    int getTabSize() const;
    IndentationStyle getIndentationStyle() const;
};

#endif