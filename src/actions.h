#ifndef ACTIONS_H
#define ACTIONS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>

#define NO_KEY -1

class Yate;
class Editor;

#define ACTION_FN_ARGS Yate& yate, Editor* editor
#define ACTION_FN [](ACTION_FN_ARGS)

typedef std::function<void(ACTION_FN_ARGS)> ActionFunction;

std::string keyToString(int key);

struct Action {
  int id;
  std::string name;
  int key;
  ActionFunction fn;

  std::string displayString;
  std::string pendingDisplayString;

  Action(int id, std::string name, int key) :
    Action(id, name, key, ActionFunction()) {}
  Action(int id, std::string name, int key, ActionFunction fn) :
    id(id), name(name), key(key), fn(fn) {
    regenerateStrings();
  }

  void regenerateStrings() {
    std::ostringstream out;
    out << getGroup() << ": " << name << " [" << keyToString(key) << "]";
    displayString = out.str();
    std::ostringstream out2;
    out2 << getGroup() << ": " << name << " [Enter Key...]";
    pendingDisplayString = out2.str();
  }
  const std::string getGroup() const;
};


struct ActionManager {
  std::vector<Action*> actions;
  std::unordered_map<int, Action*> mappings;

  static ActionManager& get() {
    static ActionManager static_inst;
    return static_inst;
  }

  void addAction(Action* action) {
    actions.push_back(action);
    mappings.emplace(action->key, action);
  }

  void changeActionKey(Action* action, int new_key) {
    mappings.erase(action->key);
    if (mappings.find(new_key) != mappings.end()) {
      mappings[new_key]->key = NO_KEY;
      mappings[new_key]->regenerateStrings();
      mappings.erase(new_key);
    }
    mappings.emplace(new_key, action);
    action->key = new_key;
    action->regenerateStrings();
  }

  bool runAction(int key, Yate& yate, Editor* editor) {
    if (mappings.find(key) != mappings.end()) {
      mappings.at(key)->fn(yate, editor);
      return true;
    }
    return false;
  }

  ActionManager();
  ~ActionManager() {
    for (auto action : actions) delete action;
  }
};

#endif
