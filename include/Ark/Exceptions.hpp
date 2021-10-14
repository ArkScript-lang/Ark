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

#include <Ark/VM/Value.hpp>

namespace Ark
{
    class Error : public std::runtime_error
    {
    public:
        explicit Error(const std::string& message = "") :
            std::runtime_error(message)
        {}
    };

    /**
     * @brief A type error triggered when types don't match
     * 
     */
    class BetterTypeError : public Error
    {
    public:
        BetterTypeError(std::string_view func_name, std::size_t expected_argc, const std::vector<Value>& args);

        BetterTypeError& withArg(std::string_view arg_name, ValueType arg_type);
        BetterTypeError& withArg(std::string_view arg_name, const std::vector<ValueType>& arg_type);

    protected:
        std::string_view m_funcname;
        std::size_t m_arg_index;
        std::size_t m_expected_argc;
        std::vector<Value> m_args;
    };

    /**
     * @brief A type error triggered when types don't match
     * 
     */
    class TypeError : public std::runtime_error
    {
    public:
        explicit TypeError(const std::string& message) :
            std::runtime_error("TypeError: " + message)
        {}
    };

    /**
     * @brief A special zero division error triggered when a number is divided by 0
     * 
     */
    class ZeroDivisionError : public std::runtime_error
    {
    public:
        ZeroDivisionError() :
            std::runtime_error(
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
    class PowError : public std::runtime_error
    {
    public:
        PowError() :
            std::runtime_error(
                "PowError: Can not pow the given number (a) to the given exponent (b) because "
                "a^b, with b being a member of the rational numbers, isn't supported.")
        {}
    };

    /**
     * @brief An assertion error, only triggered from ArkScript code through (assert expr error-message)
     * 
     */
    class AssertionFailed : public std::runtime_error
    {
    public:
        explicit AssertionFailed(const std::string& message) :
            std::runtime_error("AssertionFailed: " + message)
        {}
    };

    /**
     * @brief SyntaxError thrown by the lexer
     * 
     */
    class SyntaxError : public std::runtime_error
    {
    public:
        explicit SyntaxError(const std::string& message) :
            std::runtime_error("SyntaxError: " + message)
        {}
    };

    /**
     * @brief ParseError thrown by the parser
     * 
     */
    class ParseError : public std::runtime_error
    {
    public:
        explicit ParseError(const std::string& message) :
            std::runtime_error("ParseError: " + message)
        {}
    };

    /**
     * @brief OptimizerError thrown by the AST optimizer
     * 
     */
    class OptimizerError : public std::runtime_error
    {
    public:
        explicit OptimizerError(const std::string& message) :
            std::runtime_error("OptimizerError: " + message)
        {}
    };

    /**
     * @brief MacroProcessingError thrown by the compiler
     * 
     */
    class MacroProcessingError : public std::runtime_error
    {
    public:
        explicit MacroProcessingError(const std::string& message) :
            std::runtime_error("MacroProcessingError: " + message)
        {}
    };

    /**
     * @brief CompilationError thrown by the compiler
     * 
     */
    class CompilationError : public std::runtime_error
    {
    public:
        explicit CompilationError(const std::string& message) :
            std::runtime_error("CompilationError: " + message)
        {}
    };
}

#endif
