#include "actions.h"
#include "util.h"
#include "command-prompt.h"
#include "editor.h"
#include "filesystem-prompt.h"
#include "find-all-prompt.h"
#include "find-prompt.h"
#include "goto-line-prompt.h"
#include "tab-set.h"
#include "tags-prompt.h"
#include "resize-prompt.h"
#include "buffer-select-prompt.h"
#include "prefix-trie-prompt.h"
#include "config-prompt.h"
#include "history-prompt.h"
#include "quit-prompt.h"
#include "syntax-prompt.h"
#include "actions-prompt.h"
#include "log-pane.h"

#define DECLARE_ACTION(...) addAction(new Action(__VA_ARGS__))

#define DECLARE_GROUP(name, from, to) if (from <= id && id < to) return name;

std::string keyToString(int key) {
  if (key == NO_KEY) {
    return "None";
  }
  if (ctrl(key) == key) {
    return "C-" + std::string(1, (char)_unctrl(key));
  }
  return std::string(1, (char)key);
}
const std::string Action::getGroup() const {
  DECLARE_GROUP("File", 0, 1000);
  DECLARE_GROUP("Edit", 1000, 2000);
  DECLARE_GROUP("Navigate", 2000, 3000);
  DECLARE_GROUP("Tab", 3000, 4000);
  return "Unknown group: " + std::to_string(id);
}

ActionManager::ActionManager() {
  DECLARE_ACTION(0, "Open Command Prompt", ctrl('k'), ACTION_FN {
    CommandPromptWindow* p = new CommandPromptWindow(yate, editor);
    yate.enterPrompt(p);
  });
  DECLARE_ACTION(10, "Open File", ctrl('o'), ACTION_FN {
    yate.enterPrompt(new FileSystemWindow(
       yate, editor, editor->getBuffer()->cwd, nullptr,
       std::bind(
         static_cast<void (Editor::*)(std::string)>(&Editor::switchBuffer),
         editor, std::placeholders::_1)));
  });
  DECLARE_ACTION(20, "Open Buffer", NO_KEY, ACTION_FN {
    yate.enterPrompt(new BufferSelectPromptWindow(
      yate, std::function<void(int)>([&yate, editor](int bufferIndex) {
        editor->switchBuffer(yate.opened_buffers.at(bufferIndex));
      })
    ));
  });
  DECLARE_ACTION(30, "Save File", ctrl('s'), ACTION_FN {
    editor->getBuffer()->writeToFile();
  });
  DECLARE_ACTION(40, "Revert File", NO_KEY, ACTION_FN {
    editor->revertBuffer();
  });
  DECLARE_ACTION(50, "View Log", NO_KEY, ACTION_FN {
    editor->paneset_parent->replaceChildWith<LogPane>(editor->paneset_parent_child);
  });
  DECLARE_ACTION(60, "Actions / Keyboard Shortcuts", NO_KEY, ACTION_FN {
    yate.enterPrompt(new ActionsPromptWindow(yate));
  });
  DECLARE_ACTION(70, "Configuration", NO_KEY, ACTION_FN {
    yate.enterPrompt(new ConfigPromptWindow(yate));
  });
  DECLARE_ACTION(80, "Quit", ctrl('q'), ACTION_FN {
    yate.enterPrompt(new QuitPromptWindow(yate));
  });

  DECLARE_ACTION(1000, "Undo", ctrl('z'), ACTION_FN {
    editor->getBuffer()->undo(editor->current_line, editor->current_col);
    editor->selection_start = NO_SELECTION;
  });
  DECLARE_ACTION(1010, "Redo", ctrl('y'), ACTION_FN {
    editor->getBuffer()->redo(editor->current_line, editor->current_col);
    editor->selection_start = NO_SELECTION;
  });
  DECLARE_ACTION(1020, "View History", ctrl('h'), ACTION_FN {
    yate.enterPrompt(new HistoryPromptWindow(yate, editor));
  });
  DECLARE_ACTION(1030, "Copy", ctrl('c'), ACTION_FN {
    if (editor->selection_start == NO_SELECTION) {
      return;
    }
    LineCol current = LINECOL(editor->current_line, editor->current_col);
    yate.clipboard_buffers.push_front(
        editor->buffer->getTextInRange(current, editor->selection_start));
    // TODO(felixguo): Make this configurable?
    while (yate.clipboard_buffers.size() > 10) {
      yate.clipboard_buffers.pop_back();
    }
  });
  DECLARE_ACTION(1040, "Cut", ctrl('x'), ACTION_FN {
    if (editor->selection_start == NO_SELECTION) {
      return;
    }
    LineCol current = LINECOL(editor->current_line, editor->current_col);
    yate.clipboard_buffers.push_front(
        editor->buffer->getTextInRange(current, editor->selection_start));
    // TODO(felixguo): Make this configurable?
    while (yate.clipboard_buffers.size() > 10) {
      yate.clipboard_buffers.pop_back();
    }
    editor->deleteSelection();
  });
  DECLARE_ACTION(1050, "Paste", ctrl('v'), ACTION_FN {
    if (yate.clipboard_buffers.size() > 0) {
      editor->paste(yate.clipboard_buffers.front());
    }
  });
  DECLARE_ACTION(1060, "Select All", ctrl('a'), ACTION_FN {
    editor->selection_start = LINECOL(0, 0);
    editor->current_line = editor->buffer->size() - 1;
    editor->current_col = editor->buffer->getLineLength(editor->current_line);
  });
  DECLARE_ACTION(1070, "Find", ctrl('f'), ACTION_FN {
    yate.enterPrompt(new FindPromptWindow(yate, editor));
  });
  DECLARE_ACTION(1070, "Find Word", ctrl('j'), ACTION_FN {
    yate.enterPrompt(new FindPromptWindow(yate, editor, editor->getCurrentWord()));
  });
  DECLARE_ACTION(1080, "Find All", ctrl('d'), ACTION_FN {
    yate.enterPrompt(new FindAllPromptWindow(yate));
  });
  DECLARE_ACTION(1081, "Previous Find", ctrl('['), ACTION_FN {
    yate.enterPrompt(new FindPromptWindow(yate, editor,
      FindPromptWindow::Direction::Previous));
  });
  DECLARE_ACTION(1082, "Next Find", ctrl(']'), ACTION_FN {
    yate.enterPrompt(new FindPromptWindow(yate, editor,
      FindPromptWindow::Direction::Next));
  });

  DECLARE_ACTION(1090, "Choose Syntax", NO_KEY, ACTION_FN {
    yate.enterPrompt(new SyntaxPromptWindow(yate, editor));
  });
  DECLARE_ACTION(1095, "Delete Word", 31, ACTION_FN {
    editor->deleteWord();
  });
  DECLARE_ACTION(1096, "Delete Line", ctrl('l'), ACTION_FN {
    editor->deleteLine();
  });
  DECLARE_ACTION(1100, "Indent", '\t', ACTION_FN {
    editor->getBuffer()->setRevisionLock();
    if (editor->selection_start == NO_SELECTION) {
      editor->insertTab(editor->current_line, editor->current_col);
    } else {
      LineNumber start = std::min(LINE(editor->selection_start), editor->current_line);
      LineNumber end = std::max(LINE(editor->selection_start), editor->current_line);
      ColNumber zero = 0;
      for (LineNumber i = start; i <= end; i++) {
        editor->insertTab(i, zero);
        // Since insertTab takes reference and modifies it
        zero = 0;
      }
    }
    editor->getBuffer()->clearRevisionLock();
    editor->limitLineCol();
  });
  DECLARE_ACTION(1110, "Unindent", NO_KEY, ACTION_FN {
    editor->getBuffer()->setRevisionLock();
    if (editor->selection_start == NO_SELECTION) {
      editor->removeTab(editor->current_line, editor->current_col);
    } else {
      LineNumber start = std::min(LINE(editor->selection_start), editor->current_line);
      LineNumber end = std::max(LINE(editor->selection_start), editor->current_line);
      ColNumber zero = 0;
      for (LineNumber i = start; i <= end; i++) {
        editor->removeTab(i, zero);
        // Since insertTab takes reference and modifies it
        zero = 0;
      }
    }
    editor->getBuffer()->clearRevisionLock();
    editor->limitLineCol();
  });

  DECLARE_ACTION(1999, "View Prefix Trie", ctrl('b'), ACTION_FN {
    yate.enterPrompt(new PrefixTrieWindow(yate, editor));
  });

  DECLARE_ACTION(2000, "Choose Editor", ctrl('p'), ACTION_FN {
    yate.enterPrompt(new NavigateWindow(
      yate, (NavigateWindowProvider*)yate.getEditorNavigateProvider(),
      nullptr));
  });
  DECLARE_ACTION(2010, "Navigate by Tag", ctrl('t'), ACTION_FN {
    yate.enterPrompt(new TagsPromptWindow(yate, editor));
  });
  DECLARE_ACTION(2020, "Go to Line", ctrl('g'), ACTION_FN {
    yate.enterPrompt(new GoToLinePromptWindow(yate, editor));
  });
  DECLARE_ACTION(2030, "Select Root", NO_KEY, ACTION_FN {
    yate.enterPrompt(new NavigateWindow(yate, yate.root, nullptr));
  });
  DECLARE_ACTION(2040, "Select Current Editor", NO_KEY, ACTION_FN {
    std::vector<Pane *> parents;
    editor->findAllParents(parents);
    /* We want the root to be at the front */
    std::reverse(parents.begin(), parents.end());
    yate.enterPrompt(new NavigateWindow(yate, parents));
  });

  DECLARE_ACTION(3000, "New Tab", ctrl('n'), ACTION_FN {
    TabSet* first = editor->findFirstParent<TabSet>();
    if (first) {
      first->makeNewTab();
    }
  });
  DECLARE_ACTION(3010, "Close Tab", ctrl('w'), ACTION_FN {
    TabSet* first = editor->findFirstParent<TabSet>();
    if (first) {
      first->closeTab();
    }
  });
}
