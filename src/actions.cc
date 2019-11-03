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
#include "quit-prompt.h"
#include "syntax-prompt.h"

#define DECLARE_ACTION(...) addAction(Action(__VA_ARGS__))

#define DECLARE_GROUP(name, from, to) if (from <= id && id < to) return name;

const std::string Action::getGroup() const {
  DECLARE_GROUP("File", 0, 100);
  DECLARE_GROUP("Edit", 100, 200);
  DECLARE_GROUP("Navigate", 200, 300);
  DECLARE_GROUP("Tab", 300, 400);
  return "Unknown group: " + std::to_string(id);
}

ActionManager::ActionManager() {
  DECLARE_ACTION(0, "Open Command Prompt", ctrl('k'), ACTION_FN {
    CommandPromptWindow* p = new CommandPromptWindow(yate, editor);
    yate.enterPrompt(p);
  });
  DECLARE_ACTION(1, "Open File", ctrl('o'), ACTION_FN {
    yate.enterPrompt(new FileSystemWindow(
       yate, editor, editor->getBuffer()->cwd, nullptr,
       std::bind(
         static_cast<void (Editor::*)(std::string)>(&Editor::switchBuffer),
         editor, std::placeholders::_1)));
  });
  DECLARE_ACTION(2, "Open Buffer", NO_KEY, ACTION_FN {
    yate.enterPrompt(new BufferSelectPromptWindow(
      yate, std::function<void(int)>([&yate, editor](int bufferIndex) {
        editor->switchBuffer(yate.opened_buffers.at(bufferIndex));
      })
    ));
  });
  DECLARE_ACTION(3, "Save File", ctrl('s'), ACTION_FN {
    editor->getBuffer()->writeToFile();
  });
  DECLARE_ACTION(4, "Revert File", NO_KEY, ACTION_FN {
    editor->revertBuffer();
  });
  DECLARE_ACTION(5, "Quit", ctrl('q'), ACTION_FN {
    yate.enterPrompt(new QuitPromptWindow(yate));
  });

  DECLARE_ACTION(100, "Undo", ctrl('z'), ACTION_FN {
    editor->getBuffer()->undo(editor->current_line, editor->current_col);
    editor->selection_start = NO_SELECTION;
  });
  DECLARE_ACTION(101, "Redo", ctrl('y'), ACTION_FN {
    editor->getBuffer()->redo(editor->current_line, editor->current_col);
    editor->selection_start = NO_SELECTION;
  });
  DECLARE_ACTION(102, "Copy", ctrl('c'), ACTION_FN {
    if (editor->selection_start == NO_SELECTION) {
      return;
    }
    LineCol current = std::make_tuple(editor->current_line, editor->current_col);
    yate.clipboard_buffers.push_front(
        editor->buffer->getTextInRange(current, editor->selection_start));
    // TODO(felixguo): Make this configurable?
    while (yate.clipboard_buffers.size() > 10) {
      yate.clipboard_buffers.pop_back();
    }
  });
  DECLARE_ACTION(103, "Cut", ctrl('x'), ACTION_FN {
    if (editor->selection_start == NO_SELECTION) {
      return;
    }
    LineCol current = std::make_tuple(editor->current_line, editor->current_col);
    yate.clipboard_buffers.push_front(
        editor->buffer->getTextInRange(current, editor->selection_start));
    // TODO(felixguo): Make this configurable?
    while (yate.clipboard_buffers.size() > 10) {
      yate.clipboard_buffers.pop_back();
    }
    editor->deleteSelection();
  });
  DECLARE_ACTION(104, "Paste", ctrl('v'), ACTION_FN {
    if (yate.clipboard_buffers.size() > 0) {
      editor->paste(yate.clipboard_buffers.front());
    }
  });
  DECLARE_ACTION(105, "Select All", ctrl('a'), ACTION_FN {
    editor->selection_start = std::make_tuple(0, 0);
    editor->current_line = editor->buffer->size() - 1;
    editor->current_col = editor->buffer->getLineLength(editor->current_line);
  });
  DECLARE_ACTION(106, "Find", ctrl('f'), ACTION_FN {
    yate.enterPrompt(new FindPromptWindow(yate, editor));
  });
  DECLARE_ACTION(107, "Find All", ctrl('d'), ACTION_FN {
    yate.enterPrompt(new FindAllPromptWindow(yate));
  });
  DECLARE_ACTION(108, "Choose Syntax", NO_KEY, ACTION_FN {
    yate.enterPrompt(new SyntaxPromptWindow(yate, editor));
  });

  DECLARE_ACTION(200, "Choose Editor", ctrl('p'), ACTION_FN {
    yate.enterPrompt(new NavigateWindow(
      yate, (NavigateWindowProvider*)yate.getEditorNavigateProvider(),
      nullptr));
  });
  DECLARE_ACTION(201, "Navigate by Tag", ctrl('t'), ACTION_FN {
    yate.enterPrompt(new TagsPromptWindow(yate, editor));
  });
  DECLARE_ACTION(202, "Go to Line", ctrl('g'), ACTION_FN {
    yate.enterPrompt(new GoToLinePromptWindow(yate, editor));
  });
  DECLARE_ACTION(203, "Select Root", NO_KEY, ACTION_FN {
    yate.enterPrompt(new NavigateWindow(yate, yate.root, nullptr));
  });
  DECLARE_ACTION(204, "Select Current Editor", NO_KEY, ACTION_FN {
    std::vector<Pane *> parents;
    editor->findAllParents(parents);
    /* We want the root to be at the front */
    std::reverse(parents.begin(), parents.end());
    yate.enterPrompt(new NavigateWindow(yate, parents));
  });

  DECLARE_ACTION(300, "New Tab", ctrl('n'), ACTION_FN {
    TabSet* first = editor->findFirstParent<TabSet>();
    if (first) {
      first->makeNewTab();
    }
  });
  DECLARE_ACTION(301, "Close Tab", ctrl('w'), ACTION_FN {
    TabSet* first = editor->findFirstParent<TabSet>();
    if (first) {
      first->closeTab();
    }
  });
}
