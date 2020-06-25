#ifndef ark_lexer
#define ark_lexer

#include <vector>
#include <algorithm>
#include <utility>
#include <sstream>
#include <iomanip>

#include <Ark/Exceptions.hpp>
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

        /**
         * @brief Construct a new Token object
         * 
         */
        Token() = default;

        /**
         * @brief Construct a new Token object
         * 
         * @param type the token type
         * @param tok the token value
         * @param line the line where we found the token
         * @param col the column at which was the token
         */
        Token(TokenType type, const std::string& tok, std::size_t line, std::size_t col) :
            type(type), token(tok), line(line), col(col)
        {}

        /**
         * @brief Construct a new Token object from another one
         * 
         */
        Token(const Token&) = default;
    };

    /// List of available keywords in ArkScript
    const std::vector<std::string> keywords = {
        "if", "let", "mut", "set", "fun", "while",
        "begin", "import", "quote", "del"
    };

    const std::vector<std::string> operators = {
        "+", "-", "*", "/", "<=", ">=", "!=", "<", ">", "@", "=", "^"
    };

    /**
     * @brief The lexer, in charge of creating a list of tokens
     * 
     */
    class Lexer
    {
    public:
        /**
         * @brief Construct a new Lexer object
         * 
         * @param debug the debug level
         */
        Lexer(unsigned debug);

        /**
         * @brief Give code to tokenize and create the list of tokens
         * 
         * @param code the ArkScript code
         */
        void feed(const std::string& code);

        /**
         * @brief Return the list of tokens
         * 
         * @return const std::vector<Token>& 
         */
        const std::vector<Token>& tokens();

    private:
        unsigned m_debug;
        std::vector<Token> m_tokens;

        /**
         * @brief Helper function to determine the type of a token
         * 
         * @param value 
         * @return TokenType 
         */
        inline TokenType guessType(const std::string& value);

        /**
         * @brief Check if the value is a keyword in ArkScript
         * 
         * @param value 
         * @return true 
         * @return false 
         */
        inline bool isKeyword(const std::string& value);

        /**
         * @brief Check if the value is an operator in ArkScript
         * 
         * @param value 
         * @return true 
         * @return false 
         */
        inline bool isOperator(const std::string& value);

        /**
         * @brief Check if a control character / sequence is complete or not
         * 
         * @param sequence the sequence without the leading \\
         * @param next the next character to come, maybe, in the sequence
         * @return true 
         * @return false 
         */
        inline bool endOfControlChar(const std::string& sequence, char next);

        /**
         * @brief To throw nice lexer errors
         * 
         * @param message 
         * @param match 
         * @param line 
         * @param col 
         * @param context
         */
        inline void throwTokenizingError(const std::string& message, const std::string& match, std::size_t line, std::size_t col, const std::string& context);
    };

    #include "Lexer.inl"
}

#endif  // ark_lexer
