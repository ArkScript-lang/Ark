#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <regex>
#include <algorithm>
#include <utility>

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
        Comment,
        Shorthand,
        Mismatch
    };

    const std::vector<std::string> tokentype_string = {
        "Grouping", "String", "Number", "Operator",
        "Identifier", "Keyword", "Skip", "Comment",
        "Shorthand", "Mistmatch"
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
        "begin", "import", "quote", "del"
    };

    const std::vector<std::pair<TokenType, std::regex>> lex_regexes = {
        { TokenType::String,     std::regex("^(\"[^\"]*\")") },
        { TokenType::Number,     std::regex("^(((\\+|-)?[[:digit:]]+)([\\.|/](([[:digit:]]+)?))?)") },
        { TokenType::Operator,   std::regex("^([\\+|\\-|\\*|/|<=|>=|!=|<|>|@=|@|=|\\^])") },
        { TokenType::Identifier, std::regex("^([a-zA-Z_][a-zA-Z0-9_\\-!?']*)") },
        { TokenType::Skip,       std::regex("^([\\s]+)") },
        { TokenType::Comment,    std::regex("^(#.*)") },
        { TokenType::Shorthand,  std::regex("^(['])") },
        { TokenType::Mismatch,   std::regex("^(.)") }
    };

    class Lexer
    {
    public:
        Lexer(bool debug=false);

        void feed(const std::string& code);
        const std::vector<Token>& tokens();

    private:
        bool m_debug;
        std::vector<Token> m_tokens;

        inline bool isKeyword(const std::string& value)
        {
            return std::find(keywords.begin(), keywords.end(), value) != keywords.end();
        }

        inline void throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col)
        {
            throw std::runtime_error("TokenizingError: " + message + "\nAt " +
                Ark::Utils::toString(line) + ":" + Ark::Utils::toString(col) +
                " (" + match + ")"
            );
        }
    };
}

#endif  // ark_lexer
