/**
 * @file Common.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Common code for the compiler
 * @date 2021-10-02
 *
 * @copyright Copyright (c) 2021-2025
 *
 */

#ifndef ARK_COMPILER_COMMON_HPP
#define ARK_COMPILER_COMMON_HPP

#include <array>
#include <string_view>
#include <vector>
#include <cinttypes>
#include <Ark/Constants.hpp>

namespace Ark
{
    using bytecode_t = std::vector<uint8_t>;
}

namespace Ark::internal
{
    namespace bytecode
    {
        constexpr std::array Magic = { 'a', 'r', 'k', '\0' };
        constexpr std::array Version = {
            ARK_VERSION_MAJOR & 0xff00,
            ARK_VERSION_MAJOR & 0x00ff,
            ARK_VERSION_MINOR & 0xff00,
            ARK_VERSION_MINOR & 0x00ff,
            ARK_VERSION_PATCH & 0xff00,
            ARK_VERSION_PATCH & 0x00ff
        };
        constexpr std::size_t TimestampLength = 8;
        constexpr std::size_t HeaderSize = Magic.size() + Version.size() + TimestampLength;
    }

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
        Namespace,
        Unused
    };

    /// Node types as string, in the same order as the enum NodeType
    constexpr std::array<std::string_view, 11> nodeTypes = {
        "Symbol",
        "Capture",
        "Keyword",
        "String",
        "Number",
        "List",
        "Spread",
        "Field",
        "Macro",
        "Namespace",
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
        constexpr std::string_view SetAtInPlace = "@=";
        constexpr std::string_view SetAt2InPlace = "@@=";
        /// All the builtins that modify in place a variable
        constexpr std::array UpdateRef = {
            AppendInPlace, ConcatInPlace, PopInPlace,
            SetAtInPlace, SetAt2InPlace
        };

        // This list is related to include/Ark/Compiler/Instructions.hpp
        // The order is very important
        constexpr std::array<std::string_view, 9> listInstructions = {
            "list",
            "append",
            "concat",
            AppendInPlace,
            ConcatInPlace,
            "pop",
            PopInPlace,
            SetAtInPlace,
            SetAt2InPlace
        };

        constexpr std::string_view SysArgs = "builtin__sys:args";
        constexpr std::string_view SysProgramName = "builtin__sys:programName";

        constexpr std::string_view And = "and";
        constexpr std::string_view Or = "or";

        constexpr std::string_view Undef = "$undef";
        constexpr std::string_view Symcat = "$symcat";
        constexpr std::string_view Argcount = "$argcount";
        constexpr std::string_view Repr = "$repr";
        constexpr std::string_view AsIs = "$as-is";
        constexpr std::string_view Type = "$type";

        constexpr std::array macros = {
            Undef,
            Symcat,
            Argcount,
            Repr,
            AsIs,
            Type
        };

        // This list is related to include/Ark/Compiler/Instructions.hpp
        // from FIRST_OPERATOR, to LAST_OPERATOR
        // The order is very important
        constexpr std::array<std::string_view, 24> operators = {
            "+", "-", "*", "/",
            ">", "<", "<=", ">=", "!=", "=",
            "len", "empty?", "tail", "head",
            "nil?", "assert",
            "toNumber", "toString",
            "@", "@@", "mod",
            "type", "hasField",
            "not"
        };
    }
}

#endif
