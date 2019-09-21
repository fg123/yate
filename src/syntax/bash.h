#include "syntax-highlighting.h"

class BashSyntax : public Syntax {
 public:
  virtual ~BashSyntax() {}
  LineCol match(SyntaxHighlighting::Component component,
                std::vector<std::string> &input, LineCol start) override;
  bool isMultiline(SyntaxHighlighting::Component component) override;
  bool matchFile(Buffer* buffer) override;
};
