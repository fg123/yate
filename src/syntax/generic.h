#include "syntax-highlighting.h"

class GenericSyntax : public Syntax {
 public:
  virtual ~GenericSyntax() {}
  LineCol match(SyntaxHighlighting::Component component,
                std::vector<std::string> &input, LineCol start) override;
  bool isMultiline(SyntaxHighlighting::Component component) override;
  bool matchFile(Buffer* buffer) override;
};
