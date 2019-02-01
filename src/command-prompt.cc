#include "command-prompt.h"
#include "quit-prompt.h"
#include "buffer-select-prompt.h"
#include "filesystem-prompt.h"
#include "navigate-prompt.h"
#include "navigate-window-provider.h"
#include "pane-set.h"

CommandPromptWindow::CommandPromptWindow(Yate &yate, Editor *editor)
        : PromptWindow(yate), editor(editor) {
    items.emplace_back("File: Open", std::function<void()>([editor]() {
                            editor->onKeyPress(ctrl('o'));
                        }));
    items.emplace_back(
        "File: Open Buffer", std::function<void()>([&yate, editor]() {
            yate.enterPrompt(new BufferSelectPromptWindow(
                yate, std::function<void(int)>([&yate, editor](int bufferIndex) {
                editor->switchBuffer(yate.opened_buffers.at(bufferIndex));
                })));
        }));
    items.emplace_back("File: Save", std::function<void()>([editor]() {
                            editor->onKeyPress(ctrl('s'));
                        }));
    items.emplace_back("File: Revert", std::function<void()>([editor]() {
                            editor->revertBuffer();
                        }));
    items.emplace_back("File: Quit",
                        std::function<void()>([&yate]() {
                            yate.enterPrompt(new QuitPromptWindow(yate));
                        }));
    items.emplace_back("Edit: Undo", std::function<void()>([editor]() {
                            editor->onKeyPress(ctrl('z'));
                        }));
    items.emplace_back("Edit: Redo", std::function<void()>([editor]() {
                            editor->onKeyPress(ctrl('y'));
                        }));
    items.emplace_back(
        "Navigate: Root", std::function<void()>([&yate]() {
            yate.enterPrompt(new NavigateWindow(yate, yate.root, nullptr));
        }));
    items.emplace_back(
        "Navigate: Editors", std::function<void()>([&yate]() {
            yate.enterPrompt(new NavigateWindow(
            yate, (NavigateWindowProvider*)yate.getEditorNavigateProvider(), nullptr));
        }));
    if (DEBUG) {
        items.emplace_back("Debug: Serialize", std::function<void()>([&yate]() {
                        std::ofstream stream("serialized.yate");
                        yate.serialize(stream);
                    }));
    }
}