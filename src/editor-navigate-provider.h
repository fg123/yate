#ifndef EDITOR_NAVIGATE_PROVIDER_H
#define EDITOR_NAVIGATE_PROVIDER_H

#include <unordered_set>
#include <iterator>

#include "yate.h"
#include "navigate-window-provider.h"

class EditorNavigateProvider : public NavigateWindowProvider {
  std::vector<Editor*> editors;

 public:
  EditorNavigateProvider(Yate& yate) {
    std::unordered_set<Editor*> set_of_editors;
    for (auto buffer : yate.opened_buffers) {
      std::vector<Editor*> &new_editors = buffer->getRegisteredEditors();
      std::copy(new_editors.begin(), new_editors.end(),
                std::inserter(set_of_editors, set_of_editors.end()));
    }
    editors.assign(set_of_editors.begin(), set_of_editors.end());
  }

  virtual ~EditorNavigateProvider() { }
  size_t getNavigationItemsSize() override { return editors.size(); }

  std::string getNavigationItem(size_t index) override {
    return editors.at(index)->getTitle();
  }

  // Returns whether or not the navigation prompt is done
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    Editor *editor = editors.at(index);
    editor->focusRequested(editor);
    return true;
  }
};
#endif
