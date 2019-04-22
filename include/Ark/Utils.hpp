#ifndef ark_utils
#define ark_utils

#include <string>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <fstream>
#include <regex>

#include <Ark/Constants.hpp>
#include <termcolor.hpp>

#ifdef ARK_DEBUG
    #define MODE(c, m1, m2) std::cout << termcolor::c << m1 << termcolor::reset << m2 << std::endl;
    #define LOG(m) MODE(cyan, __FILE__ << " (" << __LINE__ << ") : ", m)
    #define WARN(m) MODE(red, __FILE__ << " (" << __LINE__ << ") : ", m)
    #define OK(m) MODE(green, __FILE__ << " (" << __LINE__ << ") : ", m)
#endif  // ARK_DEBUG

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

        inline bool isInteger(const std::string& s)
        {
            return std::regex_match(s, std::regex("^((\\+|-)?[[:digit:]]+)$"));
        }

        inline bool isFloat(const std::string& s)
        {
            return std::regex_match(s, std::regex("^((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))$"));
        }

        inline bool isFraction(const std::string& s)
        {
            return std::regex_match(s, std::regex("^((\\+|-)?[[:digit:]]+)(/(([[:digit:]]+)?))$"));
        }

        inline bool fileExists(const std::string& name)
        {
            std::ifstream f(name.c_str());
            return f.good();
        }

        inline std::string readFile(const std::string& name)
        {
            std::ifstream f(name.c_str());
            // admitting the file exists
            return std::string(
                (std::istreambuf_iterator<char>(f)),
                std::istreambuf_iterator<char>()
            );
        }
    }
}

#endif  // ark_utils