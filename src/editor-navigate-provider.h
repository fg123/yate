#ifndef EDITOR_NAVIGATE_PROVIDER_H
#define EDITOR_NAVIGATE_PROVIDER_H

#include "editor.h"
#include "navigate-window-provider.h"
#include "yate.h"

#include <algorithm>

class EditorNavigateProvider : public NavigateWindowProvider {
  Yate& yate;

  std::vector<std::string> files;

 public:
  EditorNavigateProvider(Yate &yate) : yate(yate) {
    yate.filesystemIndexer.fileSetLock.lock();
    files.insert(files.end(), yate.filesystemIndexer.files.begin(),
      yate.filesystemIndexer.files.end());
    yate.filesystemIndexer.fileSetLock.unlock();
  }

  virtual ~EditorNavigateProvider() {}
  size_t getNavigationItemsSize() override {
    return yate.editors.size() + 1 + files.size();
  }

  std::string getNavigationItem(size_t index) override {
    if (index < yate.editors.size()) {
      return yate.editors.at(index)->getTitle();
    }
    else if (index == yate.editors.size()) {
      return "====================================";
    }
    else {
      return "==>" + files[(index - yate.editors.size() - 1)];
    }
  }

  // Returns whether or not the navigation prompt is done
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    Editor *editor = yate.editors.at(index);
    editor->focusRequested(editor);
    return true;
  }
};
#endif
