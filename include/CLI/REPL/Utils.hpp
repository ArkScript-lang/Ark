/**
 * @file Utils.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief replxx utilities
 * @version 1.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef REPL_REPLXX_UTIL_HPP
#define REPL_REPLXX_UTIL_HPP

#include <vector>
#include <string>

#include <replxx.hxx>

namespace Ark::internal
{
    /**
     * @brief Count the open enclosure and its counterpart: (), {}, []
     * @param line data to operate on
     * @param open the open char: (, { or [
     * @param close the closing char: ), } or ]
     * @return positive if there are more open enclosures than closed. 0 when both are equal, negative otherwise
     */
    long countOpenEnclosures(const std::string& line, char open, char close);

    /**
     * @brief Remove whitespaces at the start and end of a string
     * @param line string modified in place
     */
    void trimWhitespace(std::string& line);

    replxx::Replxx::completions_t hookCompletion(const std::vector<std::string>& words, const std::string& context, int& length);

    void hookColor(const std::vector<std::pair<std::string, replxx::Replxx::Color>>& words_colors, const std::string& context, replxx::Replxx::colors_t& colors);

    replxx::Replxx::hints_t hookHint(const std::vector<std::string>& words, const std::string& context, int& length, replxx::Replxx::Color& color);
}

#endif
