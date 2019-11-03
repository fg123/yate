#include "command-prompt.h"
#include "buffer-select-prompt.h"
#include "filesystem-prompt.h"
#include "syntax-prompt.h"
#include "navigate-prompt.h"
#include "navigate-window-provider.h"
#include "pane-set.h"
#include "quit-prompt.h"
#include "paste-prompt.h"

CommandPromptWindow::CommandPromptWindow(Yate &yate, Editor *editor)
    : PromptWindow(yate), editor(editor) {
  std::vector<Action>& actions = ActionManager::get().actions;

  for (size_t i = 1; i < actions.size(); i++) {
    auto action = actions[i];
    items.emplace_back(action.getGroup() + ": " + action.name);
  }

  if (DEBUG) {
    items.emplace_back("Debug: Serialize");
    special_commands.emplace_back([&yate]() {
      std::ofstream stream("serialized.yate");
      yate.serialize(stream);
    });
  }
}
