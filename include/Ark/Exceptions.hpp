/**
 * @file Exceptions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com), Max (madstk1@pm.me)
 * @brief ArkScript homemade exceptions
 * @version 1.3
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef INCLUDE_ARK_EXCEPTIONS_HPP
#define INCLUDE_ARK_EXCEPTIONS_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <optional>
#include <ostream>
#include <iostream>

#include <Ark/Compiler/AST/utf8_char.hpp>
#include <Ark/Platform.hpp>

namespace Ark
{
    namespace internal
    {
        class Node;
    }

    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string& message) :
            std::runtime_error(message)
        {}
    };

    /**
     * @brief A type error triggered when types don't match
     *
     */
    class TypeError final : public Error
    {
    public:
        explicit TypeError(const std::string& message) :
            Error(message)
        {}
    };

    /**
     * @brief A special zero division error triggered when a number is divided by 0
     *
     */
    class ZeroDivisionError final : public Error
    {
    public:
        ZeroDivisionError() :
            Error(
                "ZeroDivisionError: In ordinary arithmetic, the expression has no meaning, "
                "as there is no number which, when multiplied by 0, gives a (assuming a != 0), "
                "and so division by zero is undefined. Since any number multiplied by 0 is 0, "
                "the expression 0/0 is also undefined.")
        {}
    };

    /**
     * @brief An assertion error, only triggered from ArkScript code through (assert expr error-message)
     *
     */
    class AssertionFailed final : public Error
    {
    public:
        explicit AssertionFailed(const std::string& message) :
            Error("AssertionFailed: " + message)
        {}
    };

    /**
     * @brief CodeError thrown by the compiler (parser, macro processor, optimizer, and compiler itself)
     *
     */
    struct CodeError final : Error
    {
        const std::string filename;
        const std::size_t line;
        const std::size_t col;
        const std::string expr;
        const std::optional<internal::utf8_char_t> symbol;

        CodeError(
            const std::string& what,
            std::string filename_,
            const std::size_t lineNum,
            const std::size_t column,
            std::string exp,
            const std::optional<internal::utf8_char_t> opt_sym = std::nullopt) :
            Error(what),
            filename(std::move(filename_)), line(lineNum), col(column), expr(std::move(exp)), symbol(opt_sym)
        {}
    };

    namespace Diagnostics
    {
        /**
         * @brief Helper to create a colorized context to report errors to the user
         *
         * @param os stream in which the error will be written
         * @param code content of the source file where the error is
         * @param target_line line where the error is
         * @param col_start where the error starts on the given line
         * @param sym_size bad expression that triggered the error
         * @param colorize generate colors or not
         */
        ARK_API void makeContext(std::ostream& os, const std::string& code, std::size_t target_line, std::size_t col_start, std::size_t sym_size, bool colorize);

        /**
         * @brief Helper used by the compiler to generate a colorized context from a node
         *
         * @param message error message to be included in the context
         * @param node AST node with the error
         * @return std::string
         */
        ARK_API std::string makeContextWithNode(const std::string& message, const internal::Node& node);

        /**
         * @brief Generate a diagnostic from an error and print it to the standard output
         *
         * @param e code error
         * @param os output stream
         * @param colorize generate colors or not
         */
        ARK_API void generate(const CodeError& e, std::ostream& os = std::cout, bool colorize = true);
    }
}

#endif
