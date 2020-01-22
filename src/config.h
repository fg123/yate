#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "cpptoml.h"
#include "theme.h"

#define DEFINE_TYPE(key, type, default) \
  bool _##key##_initialized = false; \
  type _##key; \
  const type& get##key() { \
    if (!_##key##_initialized) { \
      _##key##_initialized = true; \
      _##key = internal_config->get_as<type>(#key).value_or(default); \
    } \
    return _##key; \
  } \
  void set##key(type value) { \
    _##key##_initialized = true; \
    internal_config->insert(#key, value); \
    _##key = value; \
  }

#define DEFINE_BOOL(key, default) DEFINE_TYPE(key, bool, default)
#define DEFINE_INT(key, default) DEFINE_TYPE(key, int64_t, default)
#define DEFINE_ENUM(key, default) \
  bool _##key##_initialized = false; \
  key _##key; \
  const key& get##key() { \
    if (!_##key##_initialized) { \
      _##key##_initialized = true; \
      _##key = (key)internal_config->get_as<int>(#key).value_or((int)default); \
    } \
    return _##key; \
  } \
  void set##key(key value) { \
    _##key##_initialized = true; \
    internal_config->insert(#key, (int)value); \
    _##key = value; \
  }

#define DEFINE_LIST(key, type) \
  bool _##key##_initialized = false; \
  std::vector<type> _##key; \
  std::vector<type>& get##key() { \
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

enum class IndentationStyle { TAB, SPACE };
extern std::unordered_map<IndentationStyle, std::string> IndentationStyleString;

class YateConfig {
  std::shared_ptr<cpptoml::table> internal_config;

 public:
  bool wasLoadedFromFile = false;

  explicit YateConfig(std::string path);
  ~YateConfig();

  #include "config_def.h"

  Theme *getTheme() const;

  void initializeAll();
};

std::ostream &operator<<(std::ostream &output,
                         IndentationStyle style);
#endif
