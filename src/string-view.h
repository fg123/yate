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

  size_t length() const;
  size_t size() const;

  const char &operator[] (size_t) const;

  int compare(size_t pos, size_t len, const std::string& str) const;
};

#endif
