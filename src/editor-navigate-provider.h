#ifndef EDITOR_NAVIGATE_PROVIDER_H
#define EDITOR_NAVIGATE_PROVIDER_H

#include "editor.h"
#include "navigate-window-provider.h"
#include "yate.h"
#include "tab-set.h"

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
    return yate.editors.size();// + files.size();
  }

  std::string getNavigationItem(size_t index) override {
    if (index < yate.editors.size()) {
      return yate.editors.at(index)->getTitle();
    }
    else {
      return "> " + files[(index - yate.editors.size())];
    }
  }

  // Returns whether or not the navigation prompt is done
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override {
    if (index < yate.editors.size()) {
      Editor *editor = yate.editors.at(index);
      editor->focusRequested(editor);
    }
    else {
      // Open the new file in a new window
      // TODO: how to do this well
      if (Editor* editor = dynamic_cast<Editor*>(yate.getCurrentFocus())) {
        TabSet* first = editor->findFirstParent<TabSet>();
        if (first) {
          Editor* newEditor = first->makeNewTab();
          newEditor->switchBuffer(yate.getBuffer(files[index - yate.editors.size()]));
        }
      }
    }
    return true;
  }
};
#endif
