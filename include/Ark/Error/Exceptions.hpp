/**
 * @file Exceptions.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com), Max (madstk1@pm.me)
 * @brief ArkScript homemade exceptions
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
 *
 */

#ifndef ARK_ERROR_EXCEPTIONS_HPP
#define ARK_ERROR_EXCEPTIONS_HPP

#include <string>
#include <utility>
#include <stdexcept>
#include <optional>

#include <Ark/Compiler/AST/utf8_char.hpp>
#include <Ark/Utils/Platform.hpp>
#include <Ark/Error/CodeErrorContext.hpp>

namespace Ark
{
    class VM;

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

        [[nodiscard]] virtual std::string details(bool colorize [[maybe_unused]], VM& vm [[maybe_unused]]) const
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
        NestedError(const Error& e, const std::string& details, VM& vm) :
            Error("NestedError"),
            m_details(e.details(/* colorize= */ false, vm))
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
}

#endif  // ARK_ERROR_EXCEPTIONS_HPP
