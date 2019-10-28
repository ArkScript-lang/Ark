#ifndef ark_utils
#define ark_utils

#include <string>
#include <sstream>
#include <iostream>
#include <streambuf>
#include <fstream>
#include <regex>
#include <filesystem>

#include <cmath>

#include <Ark/Constants.hpp>

namespace Ark::Utils
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

    inline void stringReplaceAll(std::string& source, const std::string& from, const std::string& to)
    {
        std::string newString;
        newString.reserve(source.length());  // avoids a few memory allocations

        std::string::size_type lastPos = 0;
        std::string::size_type findPos;

        while(std::string::npos != (findPos = source.find(from, lastPos)))
        {
            newString.append(source, lastPos, findPos - lastPos);
            newString += to;
            lastPos = findPos + from.length();
        }

        // Care for the rest after last occurrence
        newString += source.substr(lastPos);

        source.swap(newString);
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
        auto temp = (std::filesystem::relative(std::filesystem::path(path))).string();
        std::replace(temp.begin(), temp.end(), '\\', '/');
        return temp;
    }

    inline bool isDouble(const std::string& s)
    {
        char* end = 0;
        double val = strtod(s.c_str(), &end);
        return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
    }
}

#endif  // ark_utils