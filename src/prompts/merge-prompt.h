#ifndef MERGE_PROMPT_H
#define MERGE_PROMPT_H

#include <sstream>

#include "prompt-window.h"
#include "util.h"

class MergePromptWindow : public PromptWindow {
  const std::string title = "Confirm pane to merge with:";

  PaneSet *paneset;
  Pane *child;
  std::vector<std::pair<Pane*, SharedEdge>> bordering_siblings;
  size_t goner_index = 0;
  NavigateWindow *window;

 public:
  MergePromptWindow(Yate &yate, PaneSet *paneset, Pane *child,
      std::vector<std::pair<Pane*, SharedEdge>> bordering_siblings, size_t goner_index,
       NavigateWindow *window) :
        PromptWindow(yate), paneset(paneset), child(child), bordering_siblings(bordering_siblings),
        goner_index(goner_index), window(window) {

  }

  const std::string &getTitle() override { return title; }
  bool match(std::string prompt_buf, size_t index) {
    std::string displayValue = getItemString(index);
    return fuzzy_match(prompt_buf, displayValue);
  }
  const std::string getItemString(size_t index) {
    Pane *pane = std::get<0>(bordering_siblings[index]);
    std::ostringstream s;
    s << pane->getTitle() << " (" << pane->x << ", " << pane->y << ", " << pane->width << ", " << pane->height << ")";
    return s.str();
  }
  const size_t getListSize() {
    return bordering_siblings.size();
  }
  void onExecute(size_t index) {
    paneset->doMerge(child, std::get<0>(bordering_siblings[index]),
      std::get<1>(bordering_siblings[index]), goner_index);
    NavigateWindow *cwindow = this->window;
    yate.exitPromptThenRun([cwindow]() { cwindow->finish(); });
  }
};
#endif
