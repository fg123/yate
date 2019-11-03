#ifndef ACTIONS_H
#define ACTIONS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#define NO_KEY -1

class Yate;
class Editor;

#define ACTION_FN_ARGS Yate& yate, Editor* editor
#define ACTION_FN [](ACTION_FN_ARGS)

typedef std::function<void(ACTION_FN_ARGS)> ActionFunction;

struct Action {
  int id;
  std::string name;
  int key;
  ActionFunction fn;

  Action(int id, std::string name, int key) :
    Action(id, name, key, ActionFunction()) {}
  Action(int id, std::string name, int key, ActionFunction fn) :
    id(id), name(name), key(key), fn(fn) {}

  const std::string getGroup() const;
};


struct ActionManager {
  std::vector<Action> actions;
  std::unordered_map<int, Action> mappings;

  static ActionManager& get() {
    static ActionManager static_inst;
    return static_inst;
  }

  void addAction(Action action) {
    actions.push_back(action);
    mappings.emplace(action.key, action);
  }

  bool runAction(int key, Yate& yate, Editor* editor) {
    if (mappings.find(key) != mappings.end()) {
      mappings.at(key).fn(yate, editor);
      return true;
    }
    return false;
  }

  ActionManager();
};

#endif
