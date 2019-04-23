#ifndef ark_utils
#define ark_utils

#include <string>
#include <sstream>
#include <iostream>

namespace Ark
{
    namespace Utils
    {
        template <typename T>
        std::string toString(const T& object)
        {
            std::ostringstream os;
            os << object;
            return os.str();
        }

        template <typename T>
        std::string toString(T&& object)
        {
            std::ostringstream os;
            os << object;
            return os.str();
        }

        inline bool isDigit(char c)
        {
            return isdigit(static_cast<unsigned char>(c)) != 0;
        }
    }
}

#endif  // ark_utils