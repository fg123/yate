#include "syntax-lookup.h"
#include <map>
#include <string>
#include "syntax/bash.h"
#include "syntax/generic.h"
std::map<std::string, Syntax*> SyntaxHighlighting::lookupMap;
void SyntaxHighlighting::initLookupMap() {
    lookupMap["bash"] = new BashSyntax;
    lookupMap["generic"] = new GenericSyntax;
}
Syntax* SyntaxHighlighting::lookup(std::string key) {
    return lookupMap.at(key);
}
std::string SyntaxHighlighting::determineSyntax(Buffer *buffer) {
    for (auto syntax : lookupMap) {
      if (syntax.first == "generic") continue;
      if (syntax.second->matchFile(buffer)) return syntax.first;
    }
    return "generic";
}
