#ifndef NAVIGATE_WINDOW_H
#define NAVIGATE_WINDOW_H

#include <string>

#include "editor.h"
#include "filesystem.h"
#include "prompt-window.h"
#include "util.h"

class NavigateWindow : public PromptWindow {
  const std::string title = "Navigate";
  NavigateWindowProvider *provider;
  NavigateWindow *parent;

 public:
  NavigateWindow(Yate &yate, std::vector<Pane *> &providers)
      : PromptWindow(yate), provider(providers.back()) {
    if (providers.size() > 1) {
      providers.pop_back();
      parent = new NavigateWindow(yate, providers);
      yate.enterPrompt(parent);
    } else {
      parent = nullptr;
    }
  }
  NavigateWindow(Yate &yate, NavigateWindowProvider *provider,
                 NavigateWindow *parent)
      : PromptWindow(yate), provider(provider), parent(parent) {}

  const std::string &getTitle() override { return title; }

  bool match(std::string buffer, size_t index) override {
    std::string displayValue = provider->getNavigationItem(index);
    return fuzzy_match(buffer, displayValue);
  }

  const std::string getItemString(size_t index) override {
    return provider->getNavigationItem(index);
  }

  void finish() {
    if (parent) {
      NavigateWindow *tmp = parent;
      yate.exitPromptThenRun([tmp]() { tmp->finish(); });
    } else {
      yate.exitPrompt();
    }
  }

  void onExecute(size_t index) override {
    if (provider->onNavigationItemSelected(index, this)) {
      finish();
    }
  }

  const size_t getListSize() { return provider->getNavigationItemsSize(); }
};

#endif
