#ifndef CONFIG_PROMPT_H
#define CONFIG_PROMPT_H

#include "prompt-window.h"
#include "util.h"
#include "edit-list-prompt.h"

#include <map>

namespace std {
  std::string to_string(bool a) {
    if (a) return "true";
    else return "false";
  }
};

// Base class for different config entries.
class ConfigEntry {
public:
  virtual bool onActivate() { return false; }
  virtual std::string getDisplayString() = 0;
  virtual void set() = 0;
  virtual void onLeft() = 0;
  virtual void onRight() = 0;
};

template<typename T>
class BaseConfigEntry : public ConfigEntry {
protected:
  T internal;
  T default_value;
  std::function<void(T)> onSet;
public:
  BaseConfigEntry(T internal, T default_value, std::function<void(T)> onSet)
    : internal(internal), default_value(default_value), onSet(onSet) { }

  std::string getDisplayString() override {
    return std::to_string(internal) + " [default: " +
           std::to_string(default_value) + "]";
  }
  void set() override { onSet(internal); }
};

class NumericalConfigEntry : public BaseConfigEntry<int64_t> {
public:
  NumericalConfigEntry(int64_t internal, int64_t default_value,
                       std::function<void(int64_t)> onSet)
    : BaseConfigEntry<int64_t>(internal, default_value, onSet) { }
  void onLeft() override { internal--; }
  void onRight() override { internal++; }
};

class BoolConfigEntry : public BaseConfigEntry<bool> {
public:
  BoolConfigEntry(bool internal, bool default_value,
                  std::function<void(bool)> onSet)
    : BaseConfigEntry<bool>(internal, default_value, onSet) { }
  void onLeft() override { internal = !internal; }
  void onRight() override { internal = !internal; }
};

class EnumConfigEntry : public NumericalConfigEntry {
  std::function<std::string(int64_t&)> getString;

public:
  EnumConfigEntry(int64_t internal, int64_t default_value,
                  std::function<void(int64_t)> onSet,
                  std::function<std::string(int64_t&)> getString)
    : NumericalConfigEntry(internal, default_value, onSet),
      getString(getString) { }

  std::string getDisplayString() override {
    return getString(internal) + " [default: " +
           getString(default_value) + "]";
  }
};

template <class T>
class ListConfigEntry : public ConfigEntry {
  std::vector<T>& internal;
  Yate& yate;

public:
  ListConfigEntry(Yate& yate, std::vector<T>& internal)
    : internal(internal), yate(yate) {}

  std::string getDisplayString() override {
    std::ostringstream s;
    s << "[";
    for (size_t i = 0; i < internal.size(); i++) {
      if (i != 0) s << ", ";
      s << internal[i];
    }
    s << "]";
    return s.str();
  }

  void set() override { }

  void onLeft() override { }

  void onRight() override { }

  bool onActivate() override {
    yate.enterPrompt(new EditListPromptWindow<T>(yate, internal));
    return true;
  }
};

class ConfigPromptWindow : public PromptWindow {
  const std::string title = "Configuration (ESC to exit):";

  std::vector<std::pair<std::string, ConfigEntry*>> entries;

 public:
  ConfigPromptWindow(Yate &yate) : PromptWindow(yate) {
    YateConfig& config = yate.config;
    #undef DEFINE_INT
    #undef DEFINE_BOOL
    #undef DEFINE_ENUM
    #undef DEFINE_LIST
    #define DEFINE_INT(key, default) \
      entries.emplace_back(#key, new NumericalConfigEntry(config.get##key(), \
        default, [&config](int64_t value){ config.set##key(value); }));
    #define DEFINE_BOOL(key, default) \
      entries.emplace_back(#key, new BoolConfigEntry(config.get##key(), \
        default, [&config](bool value){ config.set##key(value); }));
    #define DEFINE_ENUM(key, default) \
      entries.emplace_back(#key, new EnumConfigEntry((int64_t) config.get##key(), \
        (int64_t) default, [&config](int64_t value){ \
          config.set##key((key)value); \
        }, \
        [](int64_t& k) -> std::string { \
          k &= (key##String.size() - 1); \
          return key##String[(key) k]; \
        }));
    #define DEFINE_LIST(key, type) \
      entries.emplace_back(#key, new ListConfigEntry<type>(yate, config.get##key()));
    #include "config_def.h"
  }

  void onKeyPress(int key) override {
    std::vector<size_t> matched_items = get_matching_items();
    size_t index = matched_items.at(highlighted_index);
    switch (key) {
      case KEY_LEFT:
        std::get<1>(entries[index])->onLeft();
        break;
      case KEY_RIGHT:
        std::get<1>(entries[index])->onRight();
        break;
    }
    PromptWindow::onKeyPress(key);
  }

  const std::string &getTitle() override { return title; }

  bool match(std::string prompt_buf, size_t index) {
    return fuzzy_match(prompt_buf, getItemString(index));
  }

  const std::string getItemString(size_t index) {
    auto pair = entries.at(index);
    return std::get<0>(pair) + ": " + std::get<1>(pair)->getDisplayString();
  }

  const size_t getListSize() {
    return entries.size();
  }

  void onExecute(size_t index) {
    std::get<1>(entries.at(index))->onActivate();
  }
};
#endif
