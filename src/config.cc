#include "config.h"
#include "cpptoml.h"

static int tab_size;
static YateConfig::IndentationStyle indentation_style;

YateConfig::YateConfig(std::string path) {
    if (!path.empty()) {
        internal_config = cpptoml::parse_file(path);
    } else {
        internal_config = cpptoml::make_table();
    }
    tab_size =
        internal_config->get_as<int>("tab_size").value_or(4);
    indentation_style =
        (YateConfig::IndentationStyle)
            internal_config->get_as<int>("indentation_style").value_or(0);
}

int YateConfig::getTabSize() const {
    return tab_size;
}

YateConfig::IndentationStyle YateConfig::getIndentationStyle() const {
    return indentation_style;
}

std::ostream &operator<<(std::ostream &output, YateConfig::IndentationStyle style) {
    output << ((style == YateConfig::IndentationStyle::TAB) ? "Tabs" : "Spaces");
    return output;
}