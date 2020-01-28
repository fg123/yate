#ifndef UTIL_H
#define UTIL_H

#ifndef RELEASE
#define DEBUG 1
#else
#define DEBUG 0
#endif

#include <string>
#include <vector>
#include "buffer.h" // for ColNumber
// Some utility functions / macros

/* TODO(felixguo): find cross terminal for this */
/* https://gist.github.com/rkumar/1237091 */
#define KEY_SUP 337
#define KEY_SDOWN 336

#define KEY_CLEFT 553
#define KEY_CRIGHT 568
#define KEY_ESC 27

// Mask for ncurses getch keys
#define ctrl(x) ((x) & 0b00011111)

// ncurses defines their own unctrl but it's different
//   behaviour than what we want
#define _unctrl(x) ((x) | 0b01100000)

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
#include <cstring>
#include "string-view.h"

bool fuzzy_match(const std::string& needle, const std::string& haystack);
bool fuzzy_match(const std::string& needle, const std::string& haystack, ColNumber &found_position);
std::string tab_replace(const std::string& line, const std::string& reference, int tab_size, char replace_with = ' ');

template <typename R>
inline bool endsWith(const std::string& suffix, const R& str) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

template <typename R>
inline bool startsWith(const std::string& prefix, const R& str) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

inline bool fastStartsWith(const char * pfx, const StringView& str) {
  size_t l = strlen(pfx);
  if (str.size() < l) return false;
  for (size_t i = 0; i < l; i++) {
    if (str[i] != pfx[i]) {
      return false;
    }
  }
  return true;
}

inline bool fastStartsWithWordBoundary(const char * pfx, const StringView& str) {
  size_t l = strlen(pfx), i = 0;
  if (str.size() < l) return false;
  for (; i < l; i++) {
    if (str[i] != pfx[i]) {
      return false;
    }
  }
  return (i == str.size() || !std::isalnum(str[i]));
}

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

template<typename R>
inline bool startsWithWordBoundary(const std::string& needle, const R& haystack) {
  if (haystack.size() < needle.size()) return false;
  std::string::size_type i = 0;
  for (; i < needle.size(); i++) {
    if (haystack[i] != needle[i]) return false;
  }
  return (i == haystack.size() || !std::isalnum(haystack[i]));
}

inline bool isIdentifierChar(char c) {
  return std::isalpha(c) || c == '_';
}

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

#endif
