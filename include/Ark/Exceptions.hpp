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
            m_msg(message)
        {}

        virtual const char* what() const throw()
        {
            return ("TypeError: " + m_msg).c_str();
        }
    
    private:
        std::string m_msg;
    };

    class ZeroDivisionError : public std::exception
    {
    public:
        ZeroDivisionError(const std::string& message)
        {}

        virtual const char* what() const throw()
        {
            return std::string(
                "ZeroDivisionError: In ordonary arithmetic, the expression has no meaning, "
                "as there is no number which, when multiplied by 0, gives a (assuming a != 0), "
                "and so division by zero is undefined. Since any number multiplied by 0 is 0, "
                "the expression 0/0 is also undefined."
                ).c_str();
        }
    };
}

#endif