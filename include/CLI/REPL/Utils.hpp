/**
 * @file Utils.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief replxx utilities
 * @version 1.0
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef REPL_REPLXX_UTIL_HPP
#define REPL_REPLXX_UTIL_HPP

#include <vector>
#include <utility>
#include <string>

#include <replxx.hxx>

namespace Ark::internal
{
    const std::vector<std::string> KeywordsDict {
        // Keywords
        "if", "let", "mut", "set", "fun", "while", "begin", "import", "del",
        // Operators
        "len", "empty?", "tail", "head",
        "nil?", "assert", "toNumber",
        "toString", "and", "or", "mod",
        "type", "hasField", "not",
        // Constants
        "true", "false", "nil", "math:pi",
        "math:e", "math:tau", "math:Inf", "math:NaN",
        // Functions
        // List
        "append", "concat", "list", "append!", "concat!", "pop", "pop!",
        "list:reverse", "list:find", "list:slice", "list:sort", "list:fill", "list:setAt",
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
        "math:round", "math:NaN?", "math:Inf?", "math:cos",
        "math:sin", "math:tan", "math:arccos", "math:arcsin",
        "math:arctan",
        // Commands
        "quit"
    };

    const std::vector<std::pair<std::string, replxx::Replxx::Color>> ColorsRegexDict {
        // Keywords
        { "if", replxx::Replxx::Color::BRIGHTRED },
        { "let", replxx::Replxx::Color::BRIGHTRED },
        { "mut", replxx::Replxx::Color::BRIGHTRED },
        { "set", replxx::Replxx::Color::BRIGHTRED },
        { "fun", replxx::Replxx::Color::BRIGHTRED },
        { "while", replxx::Replxx::Color::BRIGHTRED },
        { "begin", replxx::Replxx::Color::BRIGHTRED },
        { "import", replxx::Replxx::Color::BRIGHTRED },
        { "quote", replxx::Replxx::Color::BRIGHTRED },
        { "del", replxx::Replxx::Color::BRIGHTRED },
        // Operators
        { "\\\"", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\-", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\+", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\=", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\/", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\*", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\<", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\>", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\!", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\[", replxx::Replxx::Color::BRIGHTBLUE },
        { "\\]", replxx::Replxx::Color::BRIGHTBLUE },
        { "@", replxx::Replxx::Color::BRIGHTBLUE },
        { "len", replxx::Replxx::Color::BRIGHTBLUE },
        { "empty\\?", replxx::Replxx::Color::BRIGHTBLUE },
        { "tail", replxx::Replxx::Color::BRIGHTBLUE },
        { "head", replxx::Replxx::Color::BRIGHTBLUE },
        { "nil\\?", replxx::Replxx::Color::BRIGHTBLUE },
        { "assert", replxx::Replxx::Color::BRIGHTBLUE },
        { "toNumber", replxx::Replxx::Color::BRIGHTBLUE },
        { "toString", replxx::Replxx::Color::BRIGHTBLUE },
        { "and", replxx::Replxx::Color::BRIGHTBLUE },
        { "or ", replxx::Replxx::Color::BRIGHTBLUE },
        { "mod", replxx::Replxx::Color::BRIGHTBLUE },
        { "type", replxx::Replxx::Color::BRIGHTBLUE },
        { "hasField", replxx::Replxx::Color::BRIGHTBLUE },
        { "not", replxx::Replxx::Color::BRIGHTBLUE },
        // Constants
        { "true", replxx::Replxx::Color::RED },
        { "false", replxx::Replxx::Color::RED },
        { "nil", replxx::Replxx::Color::RED },
        { "math:pi", replxx::Replxx::Color::BLUE },
        { "math:e", replxx::Replxx::Color::BLUE },
        { "math:tau", replxx::Replxx::Color::BLUE },
        { "math:Inf", replxx::Replxx::Color::BLUE },
        { "math:NaN", replxx::Replxx::Color::BLUE },
        // Functions
        // List
        { "append", replxx::Replxx::Color::BRIGHTGREEN },
        { "concat", replxx::Replxx::Color::BRIGHTGREEN },
        { "pop", replxx::Replxx::Color::BRIGHTGREEN },
        { "append!", replxx::Replxx::Color::BRIGHTGREEN },
        { "concat!", replxx::Replxx::Color::BRIGHTGREEN },
        { "pop!", replxx::Replxx::Color::BRIGHTGREEN },
        { "list", replxx::Replxx::Color::BRIGHTGREEN },
        { "list:reverse", replxx::Replxx::Color::BRIGHTGREEN },
        { "list:find", replxx::Replxx::Color::BRIGHTGREEN },
        { "list:slice", replxx::Replxx::Color::BRIGHTGREEN },
        { "list:sort", replxx::Replxx::Color::BRIGHTGREEN },
        { "list:fill", replxx::Replxx::Color::BRIGHTGREEN },
        { "list:setAt", replxx::Replxx::Color::BRIGHTGREEN },
        // IO
        { "print", replxx::Replxx::Color::GREEN },
        { "puts", replxx::Replxx::Color::GREEN },
        { "input", replxx::Replxx::Color::GREEN },
        { "io:writeFile", replxx::Replxx::Color::GREEN },
        { "io:readFile", replxx::Replxx::Color::GREEN },
        { "io:fileExists\\?", replxx::Replxx::Color::GREEN },
        { "io:listFiles", replxx::Replxx::Color::GREEN },
        { "io:dir\\?", replxx::Replxx::Color::GREEN },
        { "io:makeDir", replxx::Replxx::Color::GREEN },
        { "io:removeFiles", replxx::Replxx::Color::GREEN },
        // Times
        { "time", replxx::Replxx::Color::GREEN },
        // System
        { "sys:exec", replxx::Replxx::Color::GREEN },
        { "sys:sleep", replxx::Replxx::Color::GREEN },
        { "sys:exit", replxx::Replxx::Color::GREEN },
        // String
        { "str:format", replxx::Replxx::Color::BRIGHTGREEN },
        { "str:find", replxx::Replxx::Color::BRIGHTGREEN },
        { "str:removeAt", replxx::Replxx::Color::BRIGHTGREEN },
        { "str:ord", replxx::Replxx::Color::BRIGHTGREEN },
        { "str:chr", replxx::Replxx::Color::BRIGHTGREEN },
        // Mathematics
        { "math:exp", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:ln", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:ceil", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:floor", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:round", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:NaN\\?", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:Inf\\?", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:cos", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:sin", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:tan", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:arccos", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:arcsin", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:arctan", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:cosh", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:sinh", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:tanh", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:acosh", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:asinh", replxx::Replxx::Color::BRIGHTCYAN },
        { "math:atanh", replxx::Replxx::Color::BRIGHTCYAN },
        // Numbers
        { "[\\-|+]{0,1}[0-9]+(\\.[0-9]+)?", replxx::Replxx::Color::YELLOW },
        // Strings
        { "\".*?\"", replxx::Replxx::Color::BRIGHTGREEN },
        // Commands
        { "quit", replxx::Replxx::Color::BRIGHTMAGENTA }
    };


    /**
     * @brief Count the open enclosure and its counterpart: (), {}, []
     * @param line data to operate on
     * @param open the open char: (, { or [
     * @param close the closing char: ), } or ]
     * @return positive if there are more open enclosures than closed. 0 when both are equal, negative otherwise
     */
    long countOpenEnclosures(const std::string& line, char open, char close);

    /**
     * @brief Remove whitespaces at the start and end of a string
     * @param line string modified in place
     */
    void trimWhitespace(std::string& line);

    replxx::Replxx::completions_t hookCompletion(const std::string& context, int& length);

    void hookColor(const std::string& str, replxx::Replxx::colors_t& colors);

    replxx::Replxx::hints_t hookHint(const std::string& context, int& length, replxx::Replxx::Color& color);
}

#endif
