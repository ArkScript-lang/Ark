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
        Capture,
        GetField,
        Keyword,
        Skip,
        Comment,
        Shorthand,
        Mismatch
    };

    // TokenType to string
    const std::vector<std::string> tokentype_string = {
        "Grouping", "String", "Number", "Operator",
        "Identifier", "Capture", "GetField", "Keyword",
        "Skip", "Comment", "Shorthand", "Mistmatch"
    };

    struct Token
    {
        TokenType type;
        std::string token;
        std::size_t line;
        std::size_t col;

        Token() = default;

        Token(TokenType type, const std::string& tok, std::size_t line, std::size_t col) :
            type(type), token(tok), line(line), col(col)
        {}

        Token(const Token&) = default;
    };

    // list of available keywords in ArkScript
    const std::vector<std::string> keywords = {
        "if", "let", "mut", "set", "fun", "while",
        "begin", "import", "quote", "del"
    };

    const std::vector<std::pair<TokenType, std::regex>> lex_regexes = {
        { TokenType::String,     std::regex("^\"[^\"]*\"") },
        { TokenType::Number,     std::regex("^((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?") },
        { TokenType::Operator,   std::regex("^(\\+|\\-|\\*|/|<=|>=|!=|<|>|@=|@|=|\\^)") },
        { TokenType::Identifier, std::regex("^[a-zA-Z_][a-zA-Z0-9_\\-?']*") },
        { TokenType::Capture,    std::regex("^&[a-zA-Z_][a-zA-Z0-9_\\-?']*") },
        { TokenType::GetField,   std::regex("^\\.[a-zA-Z_][a-zA-Z0-9_\\-?']*") },
        { TokenType::Skip,       std::regex("^[\\s]+") },
        { TokenType::Comment,    std::regex("^#.*") },
        { TokenType::Shorthand,  std::regex("^[']") },
        { TokenType::Mismatch,   std::regex("^.") }
    };

    class Lexer
    {
    public:
        Lexer(unsigned debug);

        void feed(const std::string& code);
        const std::vector<Token>& tokens();

    private:
        unsigned m_debug;
        std::vector<Token> m_tokens;

        // helper to know if a string represents a keyword
        inline bool isKeyword(const std::string& value);
        // throwing nice tokenizing errors
        inline void throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col);
    };

    #include "Lexer.inl"
}

#endif  // ark_lexer
