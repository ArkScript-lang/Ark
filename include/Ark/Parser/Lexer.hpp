#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <regex>
#include <algorithm>
#include <unordered_map>

#include <Ark/Utils.hpp>

namespace Ark::internal
{
    enum class TokenType
    {
        Grouping,
        String,
        Number,
        Operator,
        Identifier,
        Keyword,
        Skip,
        Comment
    };

    struct Token
    {
        TokenType type;
        std::string token;
        std::size_t line;
        std::size_t col;

        Token(TokenType type, const std::string& tok, std::size_t line, std::size_t col) :
            type(type), token(tok), line(line), col(col)
        {}
    };
    
    const std::vector<std::string> keywords = {
        "if", "let", "mut", "set", "fun", "while",
        "begin", "import", "quote"
    };

    const std::unordered_map<TokenType, std::regex> lex_regexes = {
        { TokenType::Grouping,   std::regex("^(\\(\\)\\[\\]\\{\\})") },
        { TokenType::String,     std::regex("^(\"[^\"]*\")") },
        { TokenType::Number,     std::regex("^(((\\+|-)?[[:digit:]]+)([\\.|/](([[:digit:]]+)?))?)") },
        { TokenType::Operator,   std::regex("^(\\+|-|\\*|/|<=|>=|!=|<|>|@|@=|=|\\^|')") },
        { TokenType::Identifier, std::regex("^([a-zA-Z_][a-zA-Z0-9_\\-!?']*)") },
        { TokenType::Skip,       std::regex("^(\\s+)") },
        { TokenType::Comment,    std::regex("^(#.*)") }
    };

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
