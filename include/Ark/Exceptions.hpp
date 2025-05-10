/**
 * @file Exceptions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com), Max (madstk1@pm.me)
 * @brief ArkScript homemade exceptions
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2025
 *
 */

#ifndef INCLUDE_ARK_EXCEPTIONS_HPP
#define INCLUDE_ARK_EXCEPTIONS_HPP

#include <string>
#include <utility>
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

    class ARK_API Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string& message) :
            std::runtime_error(message)
        {}

        [[nodiscard]] virtual std::string details(bool colorize [[maybe_unused]]) const
        {
            return what();
        }
    };

    /**
     * @brief A type error triggered when types don't match
     *
     */
    class ARK_API TypeError final : public Error
    {
    public:
        explicit TypeError(const std::string& message) :
            Error(message)
        {}
    };

    /**
     * @brief An assertion error, only triggered from ArkScript code through (assert expr error-message)
     *
     */
    class ARK_API AssertionFailed final : public Error
    {
    public:
        explicit AssertionFailed(const std::string& message) :
            Error("AssertionFailed: " + message)
        {}
    };

    class ARK_API NestedError final : public Error
    {
    public:
        NestedError(const Error& e, const std::string& details) :
            Error("NestedError"),
            m_details(e.details(/* colorize= */ false))
        {
            if (!m_details.empty() && m_details.back() != '\n')
                m_details += '\n';
            m_details += "\n" + details;
        }

        NestedError(const std::exception& e, const std::string& details) :
            Error("NestedError"),
            m_details(e.what())
        {
            if (!m_details.empty() && m_details.back() != '\n')
                m_details += '\n';
            m_details += "\n" + details;
        }

        [[nodiscard]] const char* what() const noexcept override
        {
            return m_details.c_str();
        }

    private:
        std::string m_details;
    };

    struct ARK_API CodeErrorContext final
    {
        const std::string filename;
        const std::size_t line;
        const std::size_t col;
        const std::string expr;
        const std::optional<internal::utf8_char_t> symbol;

        CodeErrorContext(std::string filename_, const std::size_t lineNum, const std::size_t column, std::string expression, const std::optional<internal::utf8_char_t> maybe_symbol = std::nullopt) :
            filename(std::move(filename_)),
            line(lineNum),
            col(column),
            expr(std::move(expression)),
            symbol(maybe_symbol)
        {}
    };

    /**
     * @brief CodeError thrown by the compiler (parser, macro processor, optimizer, and compiler itself)
     *
     */
    struct ARK_API CodeError final : Error
    {
        const CodeErrorContext context;
        const std::optional<CodeErrorContext> additional_context;

        CodeError(const std::string& what, CodeErrorContext ctx, std::optional<CodeErrorContext> maybe_more_context = std::nullopt) :
            Error(what),
            context(std::move(ctx)),
            additional_context(std::move(maybe_more_context))
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
         * @param whole_line when true, underline the whole line, disregarding col_start and sym_size
         * @param colorize generate colors or not
         */
        ARK_API void makeContext(std::ostream& os, const std::string& code, std::size_t target_line, std::size_t col_start, std::size_t sym_size, bool whole_line, bool colorize);

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
