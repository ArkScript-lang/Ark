/**
 * @file CodeErrorContext.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Defines a code error context
 * @date 2025-08-16
 *
 * @copyright Copyright (c) 2025-2026
 *
 */

#ifndef ARK_ERROR_CODEERRORCONTEXT_HPP
#define ARK_ERROR_CODEERRORCONTEXT_HPP

#include <string>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Utils/Position.hpp>

namespace Ark
{
    struct ARK_API CodeErrorContext final
    {
        const std::string filename;
        const internal::FileSpan at;
        const bool is_macro_expansion = false;

        CodeErrorContext(std::string filename_, const internal::FileSpan& pos) :
            filename(std::move(filename_)),
            at(pos)
        {}

        CodeErrorContext(std::string filename_, const internal::FileSpan& pos, const bool from_macro_expansion) :
            filename(std::move(filename_)),
            at(pos),
            is_macro_expansion(from_macro_expansion)
        {}
    };
}

#endif  // ARK_ERROR_CODEERRORCONTEXT_HPP
