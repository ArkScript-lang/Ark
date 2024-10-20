/**
 * @file Utils.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Lots of utilities about string, filesystem and more
 * @version 1.0
 * @date 2024-07-09
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef INCLUDE_ARK_UTILS_HPP
#define INCLUDE_ARK_UTILS_HPP

#include <Ark/Platform.hpp>

#include <algorithm>
#include <string>
#include <vector>

#include <cmath>

namespace Ark::Utils
{
    /**
     * @brief Cut a string into pieces, given a character separator
     *
     * @param source
     * @param sep
     * @return std::vector<std::string>
     */
    inline std::vector<std::string> splitString(const std::string& source, const char sep)
    {
        std::vector<std::string> output;
        output.emplace_back();

        for (const char c : source)
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
        char* end = nullptr;
        const double val = strtod(s.c_str(), &end);
        if (output != nullptr)
            *output = val;
        return end != s.c_str() && *end == '\0' && val != HUGE_VAL;
    }

    /**
     * @brief Calculate the Levenshtein distance between two strings
     *
     * @param str1
     * @param str2
     * @return std::size_t
     */
    ARK_API std::size_t levenshteinDistance(const std::string& str1, const std::string& str2);
}

#endif
