#ifndef ark_utils
#define ark_utils

#include <string>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <fstream>
#include <regex>
#include <filesystem>

#include <Ark/Constants.hpp>

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
            return std::filesystem::exists(std::filesystem::path(name));
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

        inline std::string getDirectoryFromPath(const std::string& path)
        {
            return (std::filesystem::path(path)).parent_path().string();
        }

        inline std::string getFilenameFromPath(const std::string& path)
        {
            return (std::filesystem::path(path)).filename().string();
        }

        inline std::string canonicalRelPath(const std::string& path)
        {
            return (std::filesystem::relative(std::filesystem::path(path))).string();
        }
    }
}

#endif  // ark_utils