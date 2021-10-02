/**
 * @file Common.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Common code for the compiler
 * @version 0.1
 * @date 2021-10-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_COMMON_HPP
#define ARK_COMPILER_COMMON_HPP

#include <array>
#include <string_view>

namespace Ark::internal
{
    /// The different keywords available
    enum class Keyword
    {
        Fun,
        Let,
        Mut,
        Set,
        If,
        While,
        Begin,
        Import,
        Quote,
        Del
    };

    /// List of available keywords in ArkScript
    constexpr std::array<std::string_view, 10> keywords = {
        "fun",
        "let",
        "mut",
        "set",
        "if",
        "while",
        "begin",
        "import",
        "quote",
        "del"
    };

    constexpr std::array<std::string_view, 12> operators = {
        "+", "-", "*", "/",
        "<=", ">=", "!=", "<", ">",
        "@",
        "=",
        "^"
    };
}

#endif
