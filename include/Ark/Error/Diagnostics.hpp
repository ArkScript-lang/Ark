/**
 * @file Diagnostics.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Tools to report code errors nicely to the user
 * @date 2025-08-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_ERROR_DIAGNOSTICS_HPP
#define ARK_ERROR_DIAGNOSTICS_HPP

#include <string>
#include <optional>
#include <iostream>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Error/Exceptions.hpp>

namespace Ark::Diagnostics
{
    /**
     * @brief Helper to create a colorized context to report errors to the user
     *
     * @param os stream in which the error will be written
     * @param filename path to the file in which the error is
     * @param expr optional expression causing the error
     * @param sym_size length of expression to underline (can be 0)
     * @param target_line line where the error is
     * @param col_start where the error starts on the given line
     * @param maybe_context optional context, parent of the error
     * @param whole_line when true, underline the whole line, disregarding col_start and sym_size
     * @param colorize generate colors or not
     */
    ARK_API void makeContext(
        std::ostream& os,
        const std::string& filename,
        const std::optional<std::string>& expr,
        std::size_t sym_size,
        std::size_t target_line,
        std::size_t col_start,
        const std::optional<CodeErrorContext>& maybe_context,
        bool whole_line,
        bool colorize);

    /**
     * @brief Helper used by the compiler to generate a colorized context from a node
     *
     * @param message error message to be included in the context
     * @param node AST node with the error
     * @return std::string
     */
    std::string makeContextWithNode(const std::string& message, const internal::Node& node);

    /**
     * @brief Generate a diagnostic from an error and print it to the standard output
     *
     * @param e code error
     * @param os output stream
     * @param colorize generate colors or not
     */
    ARK_API void generate(const CodeError& e, std::ostream& os = std::cout, bool colorize = true);
}

#endif  // ARK_ERROR_DIAGNOSTICS_HPP
