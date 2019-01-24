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
    int tab_size() const;
    IndentationStyle indentation_style() const;
};

#endif