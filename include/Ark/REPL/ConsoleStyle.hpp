#ifndef CONSOLESTYLE_HPP
#define CONSOLESTYLE_HPP

#include <replxx.hxx>

using Replxx = replxx::Replxx;

namespace Ark
{
    const std::vector<std::string> KeywordsDict = {
        "if", "let", "mut", "set", "fun", "while",
        "begin", "import", "quote", "del", "quit",
        "true", "false", "nil", "list", "append",
        "fill", "print", "input", "readFile", "writeFile"
    };

    const std::vector<std::pair<std::string, Replxx::Color>> ColorsRegexDict = {
        // single chars
        {"\\\"", Replxx::Color::BRIGHTBLUE},
        {"\\-", Replxx::Color::BRIGHTBLUE},
        {"\\+", Replxx::Color::BRIGHTBLUE},
        {"\\=", Replxx::Color::BRIGHTBLUE},
        {"\\/", Replxx::Color::BRIGHTBLUE},
        {"\\*", Replxx::Color::BRIGHTBLUE},
        {"\\[", Replxx::Color::BRIGHTMAGENTA},
        {"\\]", Replxx::Color::BRIGHTMAGENTA},
        {"\\{", Replxx::Color::BRIGHTMAGENTA},
        {"\\}", Replxx::Color::BRIGHTMAGENTA},
        // variables
        {"let", Replxx::Color::CYAN},
        {"mut", Replxx::Color::CYAN},
        {"set", Replxx::Color::BLUE},
        {"true", Replxx::Color::BRIGHTRED},
        {"false", Replxx::Color::BRIGHTRED},
        {"nil", Replxx::Color::BRIGHTRED}, 
        // list
        {"list", Replxx::Color::BLUE},
        {"append", Replxx::Color::BLUE},
        {"fill", Replxx::Color::BLUE},
        // function
        {"fun", Replxx::Color::CYAN},
        // control flow
        {"while", Replxx::Color::MAGENTA},
        {"if", Replxx::Color::MAGENTA},
        // IO
        {"print", Replxx::Color::CYAN},
        {"input", Replxx::Color::BRIGHTGREEN},
        {"readFile", Replxx::Color::CYAN},
        {"writeFile", Replxx::Color::BRIGHTGREEN},
        // others
        {"quote", Replxx::Color::CYAN},
        {"import", Replxx::Color::BRIGHTBLUE},
        // commands
        {"quit", Replxx::Color::BRIGHTMAGENTA},
        // numbers
        {"[\\-|+]{0,1}[0-9]+", Replxx::Color::YELLOW},
        {"[\\-|+]{0,1}[0-9]*\\.[0-9]+", Replxx::Color::YELLOW},
        // strings
        {"\".*?\"", Replxx::Color::BRIGHTGREEN}
    };
}

#endif