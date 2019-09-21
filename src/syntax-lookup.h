#ifndef SYNTAX_LOOKUP_H
#define SYNTAX_LOOKUP_H

#include <map>
#include <string>

#include "syntax-highlighting.h"

namespace SyntaxHighlighting {
    extern std::map<std::string, Syntax*> lookupMap;

    void initLookupMap();
    Syntax* lookup(std::string key);
    std::string determineSyntax(Buffer *buffer);
}
#endif
