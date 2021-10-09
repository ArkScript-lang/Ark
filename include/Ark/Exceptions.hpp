/**
 * @file Exceptions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript homemade exceptions
 * @version 0.1
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

#include <termcolor/termcolor.hpp>
#include <Ark/VM/Value.hpp>

namespace Ark
{
    class Error : public std::exception
    {
    public:
        Error(std::string_view name, std::size_t eargc, const std::vector<Value>& args) :
            m_index(0), m_eargc(eargc), m_args(args)
        {}

        virtual Error& withArg(std::string view, ValueType type) = 0;

    protected:
        std::size_t m_index;
        std::size_t m_eargc;
        std::vector<Value> m_args;
    };

    /**
     * @brief A type error triggered when types don't match
     * 
     */
    class BetterTypeError : public Error
    {
    public:
        BetterTypeError(std::string_view name, std::size_t eargc, const std::vector<Value>& args) :
            Error(name, eargc, args)
        {
            std::cout << name << ": needs " << eargc << " argument(s), got " << args.size() << std::endl;
        }

        virtual BetterTypeError& withArg(std::string view, ValueType type) override
        {
            // argument has been provided
            if (m_index < m_args.size())
            {
                const ValueType prov_type = m_args[m_index].valueType();
                if (prov_type != type)
                {
                    std::cout << termcolor::yellow;
                }

                std::cout << "  -> " << view << " (" << types_to_str.at((int)type) << ") was of type " << types_to_str.at((int)prov_type) << termcolor::reset << std::endl;
            }
            // argument was not provided
            else
            {
                std::cout << termcolor::yellow;
                std::cout << "  -> " << view << " (" << types_to_str.at((int)type) << ") was not provided" << termcolor::reset << std::endl;
            }

            m_index++;
            return *this;
        }
    };

    /**
     * @brief A type error triggered when types don't match
     * 
     */
    class TypeError : public std::exception
    {
    public:
        explicit TypeError(const std::string& message) :
            m_msg("TypeError: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };

    /**
     * @brief A special zero division error triggered when a number is divided by 0
     * 
     */
    class ZeroDivisionError : public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "ZeroDivisionError: In ordonary arithmetic, the expression has no meaning, "
                   "as there is no number which, when multiplied by 0, gives a (assuming a != 0), "
                   "and so division by zero is undefined. Since any number multiplied by 0 is 0, "
                   "the expression 0/0 is also undefined.";
        }
    };

    /**
     * @brief A pow error triggered when we can't do a pow b
     * 
     */
    class PowError : public std::exception
    {
    public:
        virtual const char* what() const throw()
        {
            return "PowError: Can not pow the given number (a) to the given exponent (b) because "
                   "a^b, with b being a member of the rational numbers, isn't supported.";
        }
    };

    /**
     * @brief An assertion error, only triggered from ArkScript code through (assert expr error-message)
     * 
     */
    class AssertionFailed : public std::exception
    {
    public:
        explicit AssertionFailed(const std::string& message) :
            m_msg("AssertionFailed: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };

    /**
     * @brief SyntaxError thrown by the lexer
     * 
     */
    class SyntaxError : public std::exception
    {
    public:
        explicit SyntaxError(const std::string& message) :
            m_msg("SyntaxError: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };

    /**
     * @brief ParseError thrown by the parser
     * 
     */
    class ParseError : public std::exception
    {
    public:
        explicit ParseError(const std::string& message) :
            m_msg("ParseError: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };

    /**
     * @brief OptimizerError thrown by the AST optimizer
     * 
     */
    class OptimizerError : public std::exception
    {
    public:
        explicit OptimizerError(const std::string& message) :
            m_msg("OptimizerError: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };

    /**
     * @brief MacroProcessingError thrown by the compiler
     * 
     */
    class MacroProcessingError : public std::exception
    {
    public:
        explicit MacroProcessingError(const std::string& message) :
            m_msg("MacroProcessingError: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };

    /**
     * @brief CompilationError thrown by the compiler
     * 
     */
    class CompilationError : public std::exception
    {
    public:
        explicit CompilationError(const std::string& message) :
            m_msg("CompilationError: " + message)
        {}

        virtual const char* what() const throw()
        {
            return m_msg.c_str();
        }

    protected:
        std::string m_msg;
    };
}

#endif
