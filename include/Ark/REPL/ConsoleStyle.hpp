#ifndef CONSOLESTYLE_HPP
#define CONSOLESTYLE_HPP

#include <replxx.hxx>

using Replxx = replxx::Replxx;

namespace Ark
{
    const std::vector<std::string> KeywordsDict {
        /* Keywords */
        "if", "let", "mut", "set", 
        "fun", "while", "begin", "import", 
        "quote", "del",
        /* Operators */
        "len", "empty?", "firstOf", "tailOf",
        "headOf", "nil?", "assert", "toNumber",
        "toString", "and", "or", "mod", "type",
        "hasField", "not",
        /* Builtins */
        // Constants
        "true", "false", "nil",
        "Pi", "E", "Tau", "Inf",
        "NaN",
        // Functions
            // List
        "append", "concat", "list", "reverseList",
        "findInList", "removeAtList", "sliceList", "sort",
        "fill", "setListAt",
            // IO
        "print", "puts", "input", "writeFile",
        "readFile", "fileExists?", "listFiles", "isDir?",
        "makeDir", "removeFiles",
            // Times
        "time", "sleep",
            // System
        "system",
            // String
        "format", "findSubStr", "removeAtStr",
            // Mathematics
        "exp", "ln", "ceil", "floor",
        "round", "NaN?", "Inf?", "cos", 
        "sin", "tan", "arccos", "arcsin",
        "arctan",
        /* Commands */
        "quit"
    };

    const std::vector<std::pair<std::string, Replxx::Color>> ColorsRegexDict {
        /* Keywords */
        {"if", Replxx::Color::BRIGHTRED}, 
        {"let", Replxx::Color::BRIGHTRED}, 
        {"mut", Replxx::Color::BRIGHTRED}, 
        {"set", Replxx::Color::BRIGHTRED}, 
        {"fun", Replxx::Color::BRIGHTRED}, 
        {"while", Replxx::Color::BRIGHTRED}, 
        {"begin", Replxx::Color::BRIGHTRED}, 
        {"import", Replxx::Color::BRIGHTRED}, 
        {"quote", Replxx::Color::BRIGHTRED}, 
        {"del", Replxx::Color::BRIGHTRED},
        /* Single chars or Operators */
        // Single chars (sometine operators)
        {"\\\"", Replxx::Color::BRIGHTBLUE},
        {"\\-", Replxx::Color::BRIGHTBLUE},
        {"\\+", Replxx::Color::BRIGHTBLUE},
        {"\\=", Replxx::Color::BRIGHTBLUE},
        {"\\/", Replxx::Color::BRIGHTBLUE},
        {"\\*", Replxx::Color::BRIGHTBLUE},
        {"\\<", Replxx::Color::BRIGHTBLUE},
        {"\\>", Replxx::Color::BRIGHTBLUE},
        {"\\!", Replxx::Color::BRIGHTBLUE},
        {"\\[", Replxx::Color::BRIGHTBLUE},
        {"\\]", Replxx::Color::BRIGHTBLUE},
        // Operators
        {"len", Replxx::Color::BRIGHTBLUE},
        {"empty?", Replxx::Color::BRIGHTBLUE},
        {"firstOf", Replxx::Color::BRIGHTBLUE},
        {"tailOf", Replxx::Color::BRIGHTBLUE},
        {"headOf", Replxx::Color::BRIGHTBLUE},
        {"nil?", Replxx::Color::BRIGHTBLUE},
        {"assert", Replxx::Color::BRIGHTBLUE},
        {"toNumber", Replxx::Color::BRIGHTBLUE},
        {"toString", Replxx::Color::BRIGHTBLUE},
        {"and", Replxx::Color::BRIGHTBLUE},
        {"or", Replxx::Color::BRIGHTBLUE},
        {"mod", Replxx::Color::BRIGHTBLUE},
        {"type", Replxx::Color::BRIGHTBLUE},
        {"hasField", Replxx::Color::BRIGHTBLUE},
        {"not", Replxx::Color::BRIGHTBLUE},
        /* Builtins */
        // Constants
        {"true", Replxx::Color::RED},
        {"false", Replxx::Color::RED},
        {"nil", Replxx::Color::RED},
        {"Pi", Replxx::Color::BLUE},
        {"E", Replxx::Color::BLUE},
        {"Tau", Replxx::Color::BLUE},
        {"Inf", Replxx::Color::BLUE},
        {"NaN", Replxx::Color::BLUE},
        // Functions
            // List
        {"append", Replxx::Color::BRIGHTGREEN},
        {"concat", Replxx::Color::BRIGHTGREEN},
        {"list", Replxx::Color::BRIGHTGREEN},
        {"reverseList", Replxx::Color::BRIGHTGREEN},
        {"findInList", Replxx::Color::BRIGHTGREEN},
        {"removeAtList", Replxx::Color::BRIGHTGREEN},
        {"sliceList", Replxx::Color::BRIGHTGREEN},
        {"sort", Replxx::Color::BRIGHTGREEN},
        {"fill", Replxx::Color::BRIGHTGREEN},
        {"setListAt", Replxx::Color::BRIGHTGREEN},
            // IO
        {"print", Replxx::Color::GREEN},
        {"puts", Replxx::Color::GREEN},
        {"input", Replxx::Color::GREEN},
        {"writeFile", Replxx::Color::GREEN},
        {"readFile", Replxx::Color::GREEN},
        {"fileExists?", Replxx::Color::GREEN},
        {"listFiles", Replxx::Color::GREEN},
        {"isDir?", Replxx::Color::GREEN},
        {"makeDir", Replxx::Color::GREEN},
        {"removeFiles", Replxx::Color::GREEN},
            // Times
        {"time", Replxx::Color::GREEN},
        {"sleep", Replxx::Color::GREEN},
            // System
        {"system", Replxx::Color::GREEN},
            // String
        {"format", Replxx::Color::BRIGHTGREEN},
        {"findSubStr", Replxx::Color::BRIGHTGREEN},
        {"removeAtStr", Replxx::Color::BRIGHTGREEN},
            // Mathematics
        {"exp", Replxx::Color::BRIGHTCYAN},
        {"ln", Replxx::Color::BRIGHTCYAN},
        {"ceil", Replxx::Color::BRIGHTCYAN},
        {"floor", Replxx::Color::BRIGHTCYAN},
        {"round", Replxx::Color::BRIGHTCYAN},
        {"NaN?", Replxx::Color::BRIGHTCYAN},
        {"Inf?", Replxx::Color::BRIGHTCYAN},
        {"cos", Replxx::Color::BRIGHTCYAN}, 
        {"sin", Replxx::Color::BRIGHTCYAN},
        {"tan", Replxx::Color::BRIGHTCYAN},
        {"arccos", Replxx::Color::BRIGHTCYAN},
        {"arcsin", Replxx::Color::BRIGHTCYAN},
        {"arctan", Replxx::Color::BRIGHTCYAN},
        /* Objects */
        // Numbers
        {"[\\-|+]{0,1}[0-9]+", Replxx::Color::YELLOW},
        {"[\\-|+]{0,1}[0-9]*\\.[0-9]+", Replxx::Color::YELLOW},
        // Strings
        {"\".*?\"", Replxx::Color::BRIGHTGREEN},
        /* Commands */
        {"quit", Replxx::Color::BRIGHTMAGENTA}
    };
}

#endif