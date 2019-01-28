#ifndef UTIL_H
#define UTIL_H

#ifndef RELEASE
#define DEBUG 1
#else
#define DEBUG 0
#endif

// Some utility functions / macros

// Mask for ncurses getch keys
#define ctrl(x) ((x)&0x1f)

#define safe_exit(code) \
  do {                  \
    endwin();           \
    exit(code);         \
  } while (0)

#define UNUSED(expr) \
  do {               \
    (void)(expr);    \
  } while (0)

#include <algorithm>

inline bool fuzzy_match(std::string& needle, std::string& haystack) {
  return std::search(haystack.begin(), haystack.end(), needle.begin(),
                     needle.end(), [](char ch1, char ch2) {
                       return std::toupper(ch1) == std::toupper(ch2);
                     }) != haystack.end();
}

inline int readInt(std::istream& input) {
  int i;
  input >> i;
  return i;
}
#endif
