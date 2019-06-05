#ifndef EDITOR_NAVIGATE_PROVIDER_H
#define EDITOR_NAVIGATE_PROVIDER_H

#include <iterator>
#include <set>

#include "editor.h"
#include "navigate-window-provider.h"
#include "yate.h"

class EditorNavigateProvider : public NavigateWindowProvider {

  Yate& yate;

 public:
  EditorNavigateProvider(Yate &yate) : yate(yate) {}

  virtual ~EditorNavigateProvider() {}
  size_t getNavigationItemsSize() override { return yate.editors.size(); }

  std::string getNavigationItem(size_t index) override {
    return yate.editors.at(index)->getTitle();
  }

  // Returns whether or not the navigation prompt is done
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    Editor *editor = yate.editors.at(index);
    editor->focusRequested(editor);
    return true;
  }
};
#endif
