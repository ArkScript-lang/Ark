/**
 * @file Exceptions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com), Max (madstk1@pm.me)
 * @brief ArkScript homemade exceptions
 * @version 0.2
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef INCLUDE_ARK_EXCEPTIONS_HPP
#define INCLUDE_ARK_EXCEPTIONS_HPP

#include <exception>
#include <string>
#include <vector>
#include <stdexcept>
#include <optional>
#include <ostream>

#include <Ark/Compiler/AST/utf8_char.hpp>

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
    class TypeError : public Error
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
    class ZeroDivisionError : public Error
    {
    public:
        ZeroDivisionError() :
            Error(
                "ZeroDivisionError: In ordonary arithmetic, the expression has no meaning, "
                "as there is no number which, when multiplied by 0, gives a (assuming a != 0), "
                "and so division by zero is undefined. Since any number multiplied by 0 is 0, "
                "the expression 0/0 is also undefined.")
        {}
    };

    /**
     * @brief A pow error triggered when we can't do a pow b
     *
     */
    class PowError : public Error
    {
    public:
        PowError() :
            Error(
                "PowError: Can not pow the given number (a) to the given exponent (b) because "
                "a^b, with b being a member of the rational numbers, isn't supported.")
        {}
    };

    /**
     * @brief An assertion error, only triggered from ArkScript code through (assert expr error-message)
     *
     */
    class AssertionFailed : public Error
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
    struct CodeError : public Error
    {
        const std::string filename;
        const std::size_t line;
        const std::size_t col;
        const std::string expr;
        const std::optional<internal::utf8_char_t> symbol;

        CodeError(
            const std::string& what,
            const std::string& filename,
            std::size_t lineNum,
            std::size_t column,
            std::string exp,
            std::optional<internal::utf8_char_t> opt_sym = std::nullopt) :
            Error(what),
            filename(filename), line(lineNum), col(column), expr(std::move(exp)), symbol(opt_sym)
        {}
    };

    namespace Diagnostics
    {
        /**
         * @brief Helper to create a colorized context to report errors to the user
         *
         * @param os stream in which the error will be written
         * @param code content of the source file where the error is
         * @param line line where the error is
         * @param col_start where the error starts on the given line
         * @param sym_size bad expression that triggered the error
         */
        void makeContext(std::ostream& os, const std::string& code, std::size_t line, std::size_t col_start, std::size_t sym_size);

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
         * @param code code of the file in which the error occured
         */
        void generate(const CodeError& e, std::string code = "");
    }
}

#endif
