#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_set>

#include "cpptoml.h"
#include "theme.h"

#define DEFINE_OPTION(key, type, default) \
  bool _##key##_initialized = false; \
  type _##key; \
  const type& get##key() { \
    if (!_##key##_initialized) { \
      _##key##_initialized = true; \
      _##key = internal_config->get_as<type>(#key).value_or(default); \
    } \
    return _##key; \
  }\
  void set##key(type value) { \
    _##key##_initialized = true; \
    internal_config->insert(#key, value); \
    _##key = value; \
  }

#define DEFINE_ENUM(key, type, default) \
  bool _##key##_initialized = false; \
  type _##key; \
  const type& get##key() { \
    if (!_##key##_initialized) { \
      _##key##_initialized = true; \
      _##key = (type)internal_config->get_as<int>(#key).value_or((int)default); \
    } \
    return _##key; \
  } \
  void set##key(type value) { \
    _##key##_initialized = true; \
    internal_config->insert(#key, (int)value); \
    _##key = value; \
  }

#define DEFINE_LIST(key, type) \
  bool _##key##_initialized = false; \
  std::vector<type> _##key; \
  const std::vector<type>& get##key() { \
    if (!_##key##_initialized) { \
      auto vals = internal_config->get_array_of<type>(#key); \
      _##key##_initialized = true; \
      _##key.insert(_##key.end(), vals->begin(), vals->end()); \
    } \
    return _##key;\
  } \
  void set##key(std::vector<type> value) { \
    _##key##_initialized = true; \
    auto arr = cpptoml::make_array(); \
    for (const auto& z : value) { \
      arr->push_back(z); \
    } \
    internal_config->insert(#key, arr); \
    _##key.swap(value); \
  }

class YateConfig {
  std::shared_ptr<cpptoml::table> internal_config;

 public:
  bool wasLoadedFromFile = false;

  enum class IndentationStyle { TAB, SPACE };
  explicit YateConfig(std::string path);
  ~YateConfig();

  DEFINE_ENUM(IndentationStyle, IndentationStyle, IndentationStyle::TAB);
  DEFINE_OPTION(TrimTrailingWhitespace, bool, true);
  DEFINE_OPTION(TabSize, int, 4);
  DEFINE_LIST(ColumnMarkers, int64_t);

  Theme *getTheme() const;
};

std::ostream &operator<<(std::ostream &output,
                         YateConfig::IndentationStyle style);
#endif
