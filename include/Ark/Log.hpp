#ifndef ark_log
#define ark_log

#include <string>
#include <iostream>

#include <termcolor.hpp>

namespace Ark
{
    namespace Log
    {
        inline void error(const std::string& msg)
        {
            std::cout << termcolor::red << msg << std::endl << termcolor::reset;
        }

        template <typename T>
        void info(const T& t)
        {
            std::cout << termcolor::cyan << t << std::endl << termcolor::reset;
        }
    }
}

#endif  // ark_log
