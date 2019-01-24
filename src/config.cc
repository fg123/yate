#include "config.h"
#include "cpptoml.h"

YateConfig::YateConfig(std::string path) {
    if (!path.empty()) {
        internal_config = cpptoml::parse_file(path);
    } else {
        internal_config = cpptoml::make_table();
    }
}

int YateConfig::tab_size() const {
    return internal_config->get_as<int>("tab_size").value_or(4);
}

YateConfig::IndentationStyle YateConfig::indentation_style() const {
    return (YateConfig::IndentationStyle)internal_config->get_as<int>("indentation_style").value_or(0);
}