#include "string-view.h"

StringView::StringView(std::string& source, size_t start) :
  source(source), start(start), _length(source.length() - start) {

}

int StringView::compare(size_t pos, size_t len, const std::string& str) const {
  for (size_t i = 0; i < len; i++) {
    if (source[start + i] != str[i]) {
      return source[start + i] < str[i] ? -1 : 1;
    }
  }
  return 0;
}
