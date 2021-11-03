/**
 * @file Common.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Common code for the compiler
 * @version 0.3
 * @date 2021-10-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_COMMON_HPP
#define ARK_COMPILER_COMMON_HPP

#include <array>
#include <string_view>
#include <vector>
#include <cinttypes>

namespace Ark
{
    using bytecode_t = std::vector<uint8_t>;
}

namespace Ark::internal
{
    /// The different node types available
    enum class NodeType
    {
        Symbol,
        Capture,
        GetField,
        Keyword,
        String,
        Number,
        List,
        Closure,
        Macro,
        Spread,
        Unused
    };

    constexpr std::array<std::string_view, 11> nodeTypes = {
        "Symbol",
        "Capture",
        "GetField",
        "Keyword",
        "String",
        "Number",
        "List",
        "Closure",
        "Macro",
        "Spread",
        "Unused"
    };

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

    // This list is related to include/Ark/Compiler/Instructions.hpp
    // from FIRST_OPERATOR, to LAST_OPERATOR
    // The order is very important
    constexpr std::array<std::string_view, 25> operators = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "tail", "head",
        "nil?", "assert",
        "toNumber", "toString",
        "@", "and", "or", "mod",
        "type", "hasField",
        "not"
    };
}

#endif
