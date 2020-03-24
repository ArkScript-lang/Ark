#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <regex>
#include <algorithm>
#include <utility>

#include <Ark/Utils.hpp>
#include <Ark/Parser/Utf8Converter.hpp>

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

    const std::vector<std::pair<TokenType, std::wregex>> lex_regexes = {
        { TokenType::String,     std::wregex(utf8_to_ws(R"*(^"[^"]*")*")) },
        { TokenType::Number,     std::wregex(utf8_to_ws(R"*(^((\+|-)?[[:digit:]]+)(\.(([[:digit:]]+)?))?)*")) },
        { TokenType::Operator,   std::wregex(utf8_to_ws(R"*(^(\+|\-|\*|/|<=|>=|!=|<|>|@=|@|=|\^))*")) },
        { TokenType::Identifier, std::wregex(utf8_to_ws(R"*(^[a-zA-Z_\u0080-\uDB7F][a-zA-Z0-9_\u0080-\uDB7F\-?']*)*")) },
        { TokenType::Capture,    std::wregex(utf8_to_ws(R"*(^&[a-zA-Z_][a-zA-Z0-9_\-?']*)*")) },
        { TokenType::GetField,   std::wregex(utf8_to_ws(R"*(^\.[a-zA-Z_][a-zA-Z0-9_\-?']*)*")) },
        { TokenType::Skip,       std::wregex(utf8_to_ws(R"*(^[\s]+)*")) },
        { TokenType::Comment,    std::wregex(utf8_to_ws("^#.*")) },
        { TokenType::Shorthand,  std::wregex(utf8_to_ws("^[']")) },
        { TokenType::Mismatch,   std::wregex(utf8_to_ws("^.")) }
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
