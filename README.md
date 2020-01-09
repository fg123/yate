# yate

Yet _another_ text editor.

<p align="center" >
<img src="https://raw.githubusercontent.com/fg123/yate/master/docs/screenshot.png">
</p>

## Building and Running
```
sudo apt-get update
sudo apt-get install ncurses-dev
```

Once those are installed, run
```
make
```
and the corresponding binary will be in the `bin/` folder.

## Configuring

TODO(felixguo): setup proper configuration system

## Features

### Arbitrary Tab and Pane Support

Yate supports an arbitrary level of pane and tab layout nesting. This allows
you to create a workflow and layout that works for you within the editor. The
state will also be saved. This means you could have two side-by-side panels,
each with their own tab set, or a tab set where each tab contains of some panel
layout (and everything in between).

### Branching Edit History

When undoing and redoing, edits are saved in a branching history. Imagine you
had a base and made a change (Change 1). Then, you pressed undo, you
would now be back at the base. However, if you were to type something new,
(Change 1) would be removed in most editors, and that line of redo would be lost
forever. In Yate, a new change (Change 2) is created. You can return to
(Change 1) simply by pressing undo (bringing you back to base), then redo again.
Yate will prompt you for the redo branch to take, and you can choose (Change 1)
to apply that change.

This is analogous to a mini Git branching history and allows developers to
quickly switch between changes in a file without accidentally losing work.
