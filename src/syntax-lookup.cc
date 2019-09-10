#include "syntax-lookup.h"
#include <map>
#include <string>
#include "syntax/generic.h"
std::map<std::string, Syntax*> SyntaxHighlighting::lookupMap;
void SyntaxHighlighting::initLookupMap() {
    lookupMap["generic"] = new GenericSyntax();
}
Syntax* SyntaxHighlighting::Lookup(std::string key) {
    return lookupMap.at(key);
}
