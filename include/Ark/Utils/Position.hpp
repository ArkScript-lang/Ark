/**
 * @file Position.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Defines position utilities (for text in a file) for the parser, formatter, diagnostics
 * @date 2025-08-18
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_UTILS_POSITION_HPP
#define ARK_UTILS_POSITION_HPP

#include <optional>

#include <Ark/Utils/Platform.hpp>

namespace Ark::internal
{
    struct ARK_API FilePos
    {
        std::size_t line;    ///< 0-indexed line number
        std::size_t column;  ///< 0-indexed column number
    };

    /**
     * @brief Describes a span for a node/atom in a file, its start position and end position
     */
    struct ARK_API FileSpan
    {
        FilePos start;
        std::optional<FilePos> end;
    };
}

#endif  // ARK_UTILS_POSITION_HPP
