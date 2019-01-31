#ifndef THEME_H
#define THEME_H

#include "syntax-highlighting.h"

class Theme {
 public:
  virtual ~Theme() {}
  virtual int map(SyntaxHighlighting::Component component) = 0;
};
#endif
