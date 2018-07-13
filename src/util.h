#ifndef UTIL_H
#define UTIL_H

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

inline bool fuzzy_match(std::string& needle, std::string& haystack) {
  return std::search(haystack.begin(), haystack.end(), needle.begin(),
                     needle.end(), [](char ch1, char ch2) {
                       return std::toupper(ch1) == std::toupper(ch2);
                     }) != haystack.end();
}
#endif
