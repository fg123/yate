#include "syntax-lookup.h"
#include <map>
#include <string>
#include "syntax/bash.h"
#include "syntax/generic.h"
std::map<std::string, Syntax*> SyntaxHighlighting::lookupMap;
void SyntaxHighlighting::initLookupMap() {
    lookupMap["bash"] = new BashSyntax();
    lookupMap["generic"] = new GenericSyntax();
}
Syntax* SyntaxHighlighting::Lookup(std::string key) {
    return lookupMap.at(key);
}
