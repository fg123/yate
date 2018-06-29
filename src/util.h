#ifndef UTIL_H
#define UTIL_H

// Some utility functions / macros

// Mask for ncurses getch keys
#define ctrl(x) ((x) & 0x1f)

#define safe_exit(code) do { endwin(); exit(code); } while (0)

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif
