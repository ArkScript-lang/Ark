/**
 * @file Common.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Common code for the compiler
 * @version 0.4
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021-2024
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
        Keyword,
        String,
        Number,
        List,
        Spread,
        Field,
        Macro,
        Unused
    };

    /// Node types as string, in the same order as the enum NodeType
    constexpr std::array<std::string_view, 10> nodeTypes = {
        "Symbol",
        "Capture",
        "Keyword",
        "String",
        "Number",
        "List",
        "Spread",
        "Field",
        "Macro",
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
        Del
    };

    /// List of available keywords in ArkScript
    constexpr std::array<std::string_view, 9> keywords = {
        "fun",
        "let",
        "mut",
        "set",
        "if",
        "while",
        "begin",
        "import",
        "del"
    };

    namespace Language
    {
        constexpr std::string_view AppendInPlace = "append!";
        constexpr std::string_view ConcatInPlace = "concat!";
        constexpr std::string_view PopInPlace = "pop!";
        /// All the builtins that modify in place a variable
        constexpr std::array UpdateRef = {
            AppendInPlace, ConcatInPlace, PopInPlace
        };

        // This list is related to include/Ark/Compiler/Instructions.hpp
        // The order is very important
        constexpr std::array<std::string_view, 7> listInstructions = {
            "list",
            "append",
            "concat",
            AppendInPlace,
            ConcatInPlace,
            "pop",
            PopInPlace
        };

        constexpr std::string_view SysArgs = "sys:args";
        constexpr std::string_view SysPlatform = "sys:platform";

        constexpr std::string_view And = "and";
        constexpr std::string_view Or = "or";

        constexpr std::string_view Symcat = "$symcat";
        constexpr std::string_view Argcount = "$argcount";
        constexpr std::string_view Repr = "$repr";
        constexpr std::string_view Paste = "$paste";

        constexpr std::array macros = {
            Symcat,
            Argcount,
            Repr,
            Paste
        };

        // This list is related to include/Ark/Compiler/Instructions.hpp
        // from FIRST_OPERATOR, to LAST_OPERATOR
        // The order is very important
        constexpr std::array<std::string_view, 23> operators = {
            "+", "-", "*", "/",
            ">", "<", "<=", ">=", "!=", "=",
            "len", "empty?", "tail", "head",
            "nil?", "assert",
            "toNumber", "toString",
            "@", "mod",
            "type", "hasField",
            "not"
        };
    }
}

#endif
