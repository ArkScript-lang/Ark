/**
 * @file ConsoleStyle.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Colors per token used by replxx
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef ARK_REPL_CONSOLESTYLE_HPP
#define ARK_REPL_CONSOLESTYLE_HPP

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
        "len", "empty?", "tail", "head",
        "nil?", "assert", "toNumber",
        "toString", "and", "or", "mod",
        "type", "hasField", "not",
        /* Builtins */
        // Constants
        "true", "false", "nil", "math:pi",
        "math:e", "math:tau", "math:Inf", "math:NaN",
        // Functions
            // List
        "append", "concat", "list", "list:reverse",
        "list:find", "list:removeAt", "list:slice", "list:sort",
        "list:fill", "list:setAt",
            // IO
        "print", "puts", "input", "io:writeFile",
        "io:readFile", "io:fileExists?", "io:listFiles", "io:dir?",
        "io:makeDir", "io:removeFiles",
            // Times
        "time",
            // System
        "sys:exec", "sys:sleep",
            // String
        "str:format", "str:find", "str:removeAt",
            // Mathematics
        "math:exp", "math:ln", "math:ceil", "math:floor",
        "math:round", "math:NaN?", "Inf?", "math:cos",
        "math:sin", "math:tan", "math:arccos", "math:arcsin",
        "math:arctan",
        /* Commands */
        "quit"
    };

    const std::vector<std::pair<std::string, Replxx::Color>> ColorsRegexDict {
        /* Keywords */
        { "if", Replxx::Color::BRIGHTRED },
        { "let", Replxx::Color::BRIGHTRED },
        { "mut", Replxx::Color::BRIGHTRED },
        { "set", Replxx::Color::BRIGHTRED },
        { "fun", Replxx::Color::BRIGHTRED },
        { "while", Replxx::Color::BRIGHTRED },
        { "begin", Replxx::Color::BRIGHTRED },
        { "import", Replxx::Color::BRIGHTRED },
        { "quote", Replxx::Color::BRIGHTRED },
        { "del", Replxx::Color::BRIGHTRED },
        /* Single chars or Operators */
        // Single chars (sometine operators)
        { "\\\"", Replxx::Color::BRIGHTBLUE },
        { "\\-", Replxx::Color::BRIGHTBLUE },
        { "\\+", Replxx::Color::BRIGHTBLUE },
        { "\\=", Replxx::Color::BRIGHTBLUE },
        { "\\/", Replxx::Color::BRIGHTBLUE },
        { "\\*", Replxx::Color::BRIGHTBLUE },
        { "\\<", Replxx::Color::BRIGHTBLUE },
        { "\\>", Replxx::Color::BRIGHTBLUE },
        { "\\!", Replxx::Color::BRIGHTBLUE },
        { "\\[", Replxx::Color::BRIGHTBLUE },
        { "\\]", Replxx::Color::BRIGHTBLUE },
        // Operators
        { "len", Replxx::Color::BRIGHTBLUE },
        { "empty?", Replxx::Color::BRIGHTBLUE },
        { "tail", Replxx::Color::BRIGHTBLUE },
        { "head", Replxx::Color::BRIGHTBLUE },
        { "nil?", Replxx::Color::BRIGHTBLUE },
        { "assert", Replxx::Color::BRIGHTBLUE },
        { "toNumber", Replxx::Color::BRIGHTBLUE },
        { "toString", Replxx::Color::BRIGHTBLUE },
        { "and", Replxx::Color::BRIGHTBLUE },
        { "or ", Replxx::Color::BRIGHTBLUE },
        { "mod", Replxx::Color::BRIGHTBLUE },
        { "type", Replxx::Color::BRIGHTBLUE },
        { "hasField", Replxx::Color::BRIGHTBLUE },
        { "not", Replxx::Color::BRIGHTBLUE },
        /* Builtins */
        // Constants
        { "true", Replxx::Color::RED },
        { "false", Replxx::Color::RED },
        { "nil", Replxx::Color::RED },
        { "math:pi", Replxx::Color::BLUE },
        { "math:e", Replxx::Color::BLUE },
        { "math:tau", Replxx::Color::BLUE },
        { "math:Inf", Replxx::Color::BLUE },
        { "math:NaN", Replxx::Color::BLUE },
        // Functions
            // List
        { "append", Replxx::Color::BRIGHTGREEN },
        { "concat", Replxx::Color::BRIGHTGREEN },
        { "list", Replxx::Color::BRIGHTGREEN },
        { "list:reverse", Replxx::Color::BRIGHTGREEN },
        { "list:find", Replxx::Color::BRIGHTGREEN },
        { "list:removeAt", Replxx::Color::BRIGHTGREEN },
        { "list:slice", Replxx::Color::BRIGHTGREEN },
        { "list:sort", Replxx::Color::BRIGHTGREEN },
        { "list:fill", Replxx::Color::BRIGHTGREEN },
        { "list:setAt", Replxx::Color::BRIGHTGREEN },
            // IO
        { "print", Replxx::Color::GREEN },
        { "puts", Replxx::Color::GREEN },
        { "input", Replxx::Color::GREEN },
        { "io:writeFile", Replxx::Color::GREEN },
        { "io:readFile", Replxx::Color::GREEN },
        { "io:fileExists?", Replxx::Color::GREEN },
        { "io:listFiles", Replxx::Color::GREEN },
        { "io:dir?", Replxx::Color::GREEN },
        { "io:makeDir", Replxx::Color::GREEN },
        { "io:removeFiles", Replxx::Color::GREEN },
            // Times
        { "time", Replxx::Color::GREEN },
            // System
        { "sys:exec", Replxx::Color::GREEN },
        { "sys:sleep", Replxx::Color::GREEN },
            // String
        { "str:format", Replxx::Color::BRIGHTGREEN },
        { "str:find", Replxx::Color::BRIGHTGREEN },
        { "str:removeAt", Replxx::Color::BRIGHTGREEN },
            // Mathematics
        { "math:exp", Replxx::Color::BRIGHTCYAN },
        { "math:ln", Replxx::Color::BRIGHTCYAN },
        { "math:ceil", Replxx::Color::BRIGHTCYAN },
        { "math:floor", Replxx::Color::BRIGHTCYAN },
        { "math:round", Replxx::Color::BRIGHTCYAN },
        { "math:NaN?", Replxx::Color::BRIGHTCYAN },
        { "math:Inf?", Replxx::Color::BRIGHTCYAN },
        { "math:cos", Replxx::Color::BRIGHTCYAN },
        { "math:sin", Replxx::Color::BRIGHTCYAN },
        { "math:tan", Replxx::Color::BRIGHTCYAN },
        { "math:arccos", Replxx::Color::BRIGHTCYAN },
        { "math:arcsin", Replxx::Color::BRIGHTCYAN },
        { "math:arctan", Replxx::Color::BRIGHTCYAN },
        /* Objects */
        // Numbers
        { "[\\-|+]{0,1}[0-9]+", Replxx::Color::YELLOW },
        { "[\\-|+]{0,1}[0-9]*\\.[0-9]+", Replxx::Color::YELLOW },
        // Strings
        { "\".*?\"", Replxx::Color::BRIGHTGREEN },
        /* Commands */
        { "quit", Replxx::Color::BRIGHTMAGENTA}
    };
}

#endif
