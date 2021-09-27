/**
 * @file Utils.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Lots of utilities about string, filesystem and more
 * @version 0.2
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef INCLUDE_ARK_UTILS_HPP
#define INCLUDE_ARK_UTILS_HPP

#include <string>
#include <iostream>
#include <streambuf>
#include <fstream>
#include <filesystem>
#include <vector>

#include <cmath>

#include <Ark/Constants.hpp>

namespace Ark::Utils
{
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

        while (std::string::npos != (findPos = source.find(from, lastPos)))
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
     * @brief Cut a string into pieces, given a character separator
     * 
     * @param source 
     * @param sep 
     * @return std::vector<std::string> 
     */
    inline std::vector<std::string> splitString(const std::string& source, char sep)
    {
        std::vector<std::string> output;
        output.emplace_back();

        for (char c : source)
        {
            if (c != sep)
                output.back() += c;
            else
                output.emplace_back();  // add empty string
        }

        return output;
    }

    /**
     * @brief Checks if a string is a valid double
     * 
     * @param s the string
     * @param output optional pointer to the output to avoid 2 conversions
     * @return true on success
     * @return false on failure
     */
    inline bool isDouble(const std::string& s, double* output = nullptr)
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
    inline bool fileExists(const std::string& name) noexcept
    {
        try
        {
            return std::filesystem::exists(std::filesystem::path(name));
        }
        catch (const std::filesystem::filesystem_error&)
        {
            // if we met an error than we most likely fed an invalid path
            return false;
        }
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
            std::istreambuf_iterator<char>());
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

    /**
     * @brief Count the number of decimals for a double
     * 
     * @param d 
     * @return int 
     */
    int decPlaces(double d);

    /**
     * @brief Count the number of digits for a double
     * 
     * @param d 
     * @return int 
     */
    int digPlaces(double d);
}

#endif
