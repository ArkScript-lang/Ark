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
    }
}

#endif  // ark_log