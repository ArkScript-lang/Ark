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
#include <Ark/Utils/Position.hpp>
#include <Ark/Error/Exceptions.hpp>

namespace Ark::Diagnostics
{
    struct ErrorLocation
    {
        std::string filename;  ///< Complete path to the file where the error is
        internal::FilePos start;
        std::optional<internal::FilePos> end;

        [[nodiscard]] bool wholeLineIsError() const
        {
            return !end.has_value();
        }
    };

    /**
     * @brief Helper to create a colorized context to report errors to the user
     *
     * @param loc error location
     * @param os stream in which the error will be written
     * @param expr optional expression causing the error
     * @param maybe_context optional context, parent of the error
     * @param colorize generate colors or not
     */
    ARK_API void makeContext(
        ErrorLocation loc,
        std::ostream& os,
        const std::optional<std::string>& expr,
        const std::optional<CodeErrorContext>& maybe_context,
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
