#ifndef NAVIGATE_WINDOW_PROVIDER_H
#define NAVIGATE_WINDOW_PROVIDER_H

class NavigateWindow;
struct NavigateWindowProvider {
  virtual size_t getNavigationItemsSize() = 0;
  virtual std::string getNavigationItem(size_t index) = 0;

  // Returns whether or not the navigation prompt is done
  virtual bool onNavigationItemSelected(size_t index,
                                        NavigateWindow *parent) = 0;
};
#endif