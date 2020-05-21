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
    /**
     * @brief Home made version of std::to_string to convert to string anything which has an operator<< for this
     * 
     * @tparam T the type of the object
     * @param object the object to convert to string
     * @return std::string 
     */
    template <typename T>
    std::string toString(T&& object)
    {
        std::ostringstream os;
        os << object;
        return os.str();
    }

    /**
     * @brief Replaces all occurence of `from` to `to` in `source`
     * 
     * @param source the string in which we're going to operate
     * @param from the word to replace
     * @param to the word replacing the old one
     */
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

    /**
     * @brief Checks if a string represents a valid integer
     * 
     * @param s the string
     * @return true on success
     * @return false on failure
     */
    inline bool isInteger(const std::string& s)
    {
        return std::regex_match(s, std::regex("^((\\+|-)?[[:digit:]]+)$"));
    }

    /**
     * @brief Checks if a string represents a valid floating point number
     * 
     * @param s the string
     * @return true on success
     * @return false on failure
     */
    inline bool isFloat(const std::string& s)
    {
        return std::regex_match(s, std::regex("^((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))$"));
    }

    /**
     * @brief Checks if a string represents a valid fractional number
     * 
     * @param s the string
     * @return true on success
     * @return false on failure
     */
    inline bool isFraction(const std::string& s)
    {
        return std::regex_match(s, std::regex("^((\\+|-)?[[:digit:]]+)(/(([[:digit:]]+)?))$"));
    }

    /**
     * @brief Checks if a string is a valid double
     * 
     * @param s the string
     * @param output optional pointer to the output to avoid 2 conversions
     * @return true on success
     * @return false on failure
     */
    inline bool isDouble(const std::string& s, double* output=nullptr)
    {
        char* end = 0;
        double val = strtod(s.c_str(), &end);
        if (output != nullptr)
            *output = val;
        return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
    }

    /**
     * @brief Checks if a file exists
     * 
     * @param name the file name
     * @return true on success
     * @return false on failure
     */
    inline bool fileExists(const std::string& name)
    {
        return std::filesystem::exists(std::filesystem::path(name));
    }

    /**
     * @brief Helper to read a file
     * 
     * @param name the file name
     * @return std::string 
     */
    inline std::string readFile(const std::string& name)
    {
        std::ifstream f(name.c_str());
        // admitting the file exists
        return std::string(
            (std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()
        );
    }

    /**
     * @brief Get the directory from a path
     * 
     * @param path 
     * @return std::string 
     */
    inline std::string getDirectoryFromPath(const std::string& path)
    {
        return (std::filesystem::path(path)).parent_path().string();
    }

    /**
     * @brief Get the filename from a path
     * 
     * @param path 
     * @return std::string 
     */
    inline std::string getFilenameFromPath(const std::string& path)
    {
        return (std::filesystem::path(path)).filename().string();
    }

    /**
     * @brief Get the canonical relative path from a path
     * 
     * @param path 
     * @return std::string 
     */
    inline std::string canonicalRelPath(const std::string& path)
    {
        auto temp = (std::filesystem::relative(std::filesystem::path(path))).string();
        std::replace(temp.begin(), temp.end(), '\\', '/');
        return temp;
    }
}

#endif  // ark_utils