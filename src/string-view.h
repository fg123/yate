// A lacking implementation of a string view
#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <string>

class StringView {
  std::string& source;
  size_t start;
  size_t _length;

public:
  StringView(std::string& source, size_t start);

  inline size_t length() const { return _length;  }
  inline size_t size() const { return _length;  }

  inline const char &operator[] (size_t pos) const {
    return source[start + pos];
  }

  int compare(size_t pos, size_t len, const std::string& str) const;
};

#endif
