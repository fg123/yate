#ifndef CONFIG_PROMPT_H
#define CONFIG_PROMPT_H

#include "prompt-window.h"
#include "util.h"

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
  virtual std::string getDisplayString() = 0;
  virtual void onLeft() = 0;
  virtual void onRight() = 0;
  virtual void set() = 0;
};

template<typename T>
class GenericConfigEntry : public ConfigEntry {
  T internal;
  std::function<void(T)> onSet;
public:
  GenericConfigEntry(T internal, std::function<void(T)> onSet) : internal(internal), onSet(onSet) {

  }

  std::string getDisplayString() override {
    return std::to_string(internal);
  }

  void onLeft() override {
    internal--;
  }

  void onRight() override {
    internal++;
  }

  void set() override {
    onSet(internal);
  }
};

class ConfigPromptWindow : public PromptWindow {
  const std::string title = "Configuration:";

  std::vector<std::pair<std::string, ConfigEntry*>> entries;

 public:
  ConfigPromptWindow(Yate &yate) : PromptWindow(yate) {
    YateConfig& config = yate.config;
    #define DEFINE_LIST(key, type) //\
      entries.emplace_back(#key, std::string(config.get##key().begin(), config.get##key().end()));
    #define DEFINE_ENUM(key, type, default) //\
      entries.emplace_back(#key, std::to_string((int)config.get##key()));
    #define DEFINE_OPTION(key, type, default) \
      entries.emplace_back(#key, new GenericConfigEntry<type>(config.get##key(), [&config](type value){ config.set##key(value); }));
    #include "config_def.h"
    #undef DEFINE_LIST
    #undef DEFINE_ENUM
    #undef DEFINE_OPTION
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
    yate.exitPrompt();
  }
};
#endif
