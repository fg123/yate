#ifndef UTIL_H
#define UTIL_H

#ifndef RELEASE
#define DEBUG 1
#else
#define DEBUG 0
#endif

#include <string>
#include <vector>

// Some utility functions / macros

// Mask for ncurses getch keys
#define ctrl(x) ((x)&0x1f)

#define safe_exit(code, message)            \
  do {                                      \
    endwin();                               \
    std::cerr << message << std::endl;      \
    Logging::error << message << std::endl; \
    exit(code);                             \
  } while (0)

#define UNUSED(expr) \
  do {               \
    (void)(expr);    \
  } while (0)

#include <algorithm>

bool fuzzy_match(std::string& needle, std::string& haystack);

template <typename T>
inline T read(std::istream& input) {
  T i;
  input >> i;
  return i;
}

template <typename T>
inline int indexOf(std::vector<T> vector, T item) {
  auto it = std::find(vector.begin(), vector.end(), item);
  if (it == vector.end()) {
    return -1;
  } else {
    return std::distance(vector.begin(), it);
  }
}

inline bool startsWith(std::string needle, std::string haystack) {
  if (haystack.size() < needle.size()) return false;
  for (std::string::size_type i = 0; i < needle.size(); i++) {
    if (haystack[i] != needle[i]) return false;
  }
  return true;
}

inline bool startsWithWordBoundary(std::string needle, std::string haystack) {
  if (haystack.size() < needle.size()) return false;
  std::string::size_type i = 0;
  for (; i < needle.size(); i++) {
    if (haystack[i] != needle[i]) return false;
  }
  return (i == haystack.size() || !std::isalnum(haystack[i]));
}
#endif
