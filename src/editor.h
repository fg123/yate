// Represents a file editor.
#ifndef EDITOR_H
#define EDITOR_H

#include <fstream>
#include <string>
#include <vector>
#include "actions.h"
#include "buffer.h"
#include "focusable.h"
#include "logging.h"
#include "pane-set.h"
#include "pane.h"
#include "yate.h"

const LineCol NO_SELECTION = LINECOL(-1, -1);

class Editor : public Pane, public Focusable {
  friend class ActionManager;

protected:
  Yate &yate;
  Buffer *buffer;
  LineNumber current_line = 0;
  ColNumber current_col = 0;
  LineCol selection_start = NO_SELECTION;
  uint window_start_line = 0;
  uint window_start_col = 0;

  // For when you move cursor past an empty line
  ColNumber phantom_col_pos = 0;
  void updateColWithPhantom();
  void limitLineCol();
  void init();

  ColNumber getActualColPosition(const LineNumber &line, const ColNumber &col);
  ColNumber getActualColPosition();

  bool inSelection(LineNumber line, ColNumber col);
  void switchBuffer(std::string newPath);
  std::string generateStatusBar();

  /* Set to the closest pane_set parent */
  PaneSet *paneset_parent = nullptr;

  /* Set to the child of the aforementioned
   * pane_set that holds this instance of the editor */
  Pane *paneset_parent_child = nullptr;

  /* Keeps track of where the "currentWord" was last recorded
   * and invalidates if needed */
  LineNumber current_word_line;
  ColNumber current_word_col;
  std::string current_word;
  TrieNode *current_node;
  bool is_at_end_of_word;

  std::vector<std::string> suggested_complete;
  size_t suggested_complete_index;

  void invalidate_current_word();

  bool isReadonly = false;

 public:
  std::string lastSearch = "";
  size_t lastSearchIndex = 0;

  Editor(Yate &yate, Pane *parent, int x, int y, int width,
         int height)
      : Pane(parent, x, y, width, height), yate(yate), buffer(yate.getBuffer("Untitled")) {
    init();
  }
  Editor(Yate &yate, Pane *parent, Buffer *buffer, int x, int y, int width,
         int height)
      : Pane(parent, x, y, width, height), yate(yate), buffer(buffer) {
    init();
  }

  Editor(Yate &yate, Pane *parent, Buffer *buffer, std::istream &saved_state)
      : Pane(parent, saved_state), yate(yate), buffer(buffer) {
    Logging::breadcrumb("Deserializing Editor");
    init();
  }

  Editor(Yate &yate, Pane *parent, std::istream &saved_state)
      : Pane(parent, saved_state),
        yate(yate),
        buffer(yate.getBuffer(read<std::string>(saved_state))),
        current_line(read<int>(saved_state)),
        current_col(read<int>(saved_state)) {
    init();
  }

  ~Editor();

  virtual void switchBuffer(Buffer *newBuffer);
  void revertBuffer();

  void draw() override;
  const std::string &getTitle() override;
  Focusable *getCurrentFocus() override { return this; }
  Buffer *getBuffer() { return buffer; }

  void goToLineOffset(int offset, bool shouldMoveLineToCenter = true);

  void goToLine(LineNumber n, bool shouldMoveLineToCenter = true);
  void goToLineCol(LineNumber l, ColNumber c,
                   bool shouldMoveLineToCenter = true);
  bool shouldShowAutoComplete();

  int capture() override;
  void onKeyPress(int key) override;
  void onMouseEvent(MEVENT *event) override;
  void serialize(std::ostream &stream) override {
    stream << "editor " << x << " " << y << " " << width << " " << height << " "
           << buffer->getPath() << " " << current_line << " " << current_col;
    stream << std::endl;
  }
  size_t getNavigationItemsSize() override {
    /* Since the root of yate is a PaneSet, we can expect paneset_parent to
     * always be set. In this case there's always a child and we can always
     * split */
    return 5;
  }
  std::string getNavigationItem(size_t index) override {
    switch (index) {
      case 0:
        return "Focus";
      case 1:
        return "Split Vertically";
      case 2:
        return "Split Horizontally";
      case 3:
        return "Merge Pane (will destroy this one)";
      case 4:
        return "Resize Pane";
    }
    return "";
  }
  bool onNavigationItemSelected(size_t index, NavigateWindow *parent) override;
  void paste(std::string &str);
  void insertTab(LineNumber &line, ColNumber &col);
  void removeTab(LineNumber &line, ColNumber &col);
  void deleteSelection();
  void deleteWord();
  void deleteLine();
  void addTag(std::string label);
  void fastTravel(EditNode *to);
  const std::string & getCurrentWord() const { return current_word; }
};

#endif
