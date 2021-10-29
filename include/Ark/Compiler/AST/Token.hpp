/**
 * @file Token.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Token definition for ArkScript
 * @version 0.1
 * @date 2021-10-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_AST_TOKEN_HPP
#define ARK_COMPILER_AST_TOKEN_HPP

#include <array>
#include <string>
#include <string_view>

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
        Spread,
        Mismatch
    };

    // TokenType to string
    constexpr std::array<std::string_view, 13> tokentype_string = {
        "Grouping",
        "String",
        "Number",
        "Operator",
        "Identifier",
        "Capture",
        "GetField",
        "Keyword",
        "Skip",
        "Comment",
        "Shorthand",
        "Spread",
        "Mistmatch"
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
        Token(TokenType type, const std::string& tok, std::size_t line, std::size_t col) noexcept :
            type(type), token(tok), line(line), col(col)
        {}
    };
}

#endif
