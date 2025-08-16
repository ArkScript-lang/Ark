/**
 * @file CodeErrorContext.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Defines a code error context
 * @date 2025-08-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_ERROR_CODEERRORCONTEXT_HPP
#define ARK_ERROR_CODEERRORCONTEXT_HPP

#include <string>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/AST/utf8_char.hpp>

namespace Ark
{
    struct ARK_API ContextPosition final
    {
        std::size_t line;
        std::size_t column;

        // todo: add end position?
    };

    struct ARK_API CodeErrorContext final
    {
        const std::string filename;
        const ContextPosition at;
        const std::string expr;
        const std::optional<internal::utf8_char_t> symbol;
        const bool is_macro_expansion = false;

        // TODO: create an aggregate for line/column (start+end)
        CodeErrorContext(std::string filename_, const ContextPosition pos, std::string expression, const std::optional<internal::utf8_char_t> maybe_symbol = std::nullopt) :
            filename(std::move(filename_)),
            at(pos),
            expr(std::move(expression)),
            symbol(maybe_symbol)
        {}

        CodeErrorContext(std::string filename_, const ContextPosition pos, std::string expression, const bool from_macro_expansion) :
            filename(std::move(filename_)),
            at(pos),
            expr(std::move(expression)),
            symbol(std::nullopt),
            is_macro_expansion(from_macro_expansion)
        {}
    };
}

#endif  // ARK_ERROR_CODEERRORCONTEXT_HPP
