#include "syntax-highlighting.h"

class GenericSyntax : public Syntax {
 public:
  virtual ~GenericSyntax() {}
  ColNumber match(SyntaxHighlighting::Component component, std::string &input,
                  ColNumber start) override;
};
