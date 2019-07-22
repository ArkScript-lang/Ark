#ifndef ark_exceptions
#define ark_exceptions

#include <exception>
#include <string>

namespace Ark
{
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
}

#endif