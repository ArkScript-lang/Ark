#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <regex>

#include <Ark/Utils.hpp>

namespace Ark::internal
{
    struct Token
    {
        std::string token;
        std::size_t line;
        std::size_t col;

        Token(const std::string& tok, std::size_t line, std::size_t col) :
            token(tok), line(line), col(col)
        {}
    };

    const std::regex lexer_regex("^"
        "([\\(\\)\\[\\]\\{\\}])|"  // grouping
        "(\"[^\"]*\")|"  // strings
        "(((\\+|-)?[[:digit:]]+)([\\.|/](([[:digit:]]+)?))?)|"  // numbers
        "(\\+|-|\\*|/|<=|>=|!=|<|>|=|\\^|@)|"  // operators
        "([a-zA-Z_][a-zA-Z0-9_\\-!?']*)|"  // identifiers
        "(\\s+)|"  // whitespaces
        "(#.*)|"  // comments
        "(')"  // quoting
    );

    class Lexer
    {
    public:
        Lexer(bool debug=false);

        void feed(const std::string& code);
        bool check();

        const std::vector<Token>& tokens();

    private:
        bool m_debug;
        std::vector<Token> m_tokens;
    };
}

#endif  // ark_lexer
