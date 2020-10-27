/**
 * @file Exceptions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief ArkScript homemade exceptions
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ark_exceptions
#define ark_exceptions

#include <exception>
#include <string>

namespace Ark
{
    /**
     * @brief A type error triggered when types don't match
     * 
     */
    class TypeError : public std::exception
    {
    public:
        TypeError(const std::string& message) :
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
            return 
                "ZeroDivisionError: In ordonary arithmetic, the expression has no meaning, "
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
            return
                "PowError: Can not pow the given number (a) to the given exponent (b) because "
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
        AssertionFailed(const std::string& message) :
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
        SyntaxError(const std::string& message) :
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
        ParseError(const std::string& message) :
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
     * @brief CompilationError thrown by the compiler
     * 
     */
    class CompilationError : public std::exception
    {
    public:
        CompilationError(const std::string& message) :
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