/**
 * @file Utils.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief replxx utilities
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
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
     * @brief Compute a list of all the language keywords and builtins
     *
     * @return std::vector<std::string>
     */
    std::vector<std::string> getAllKeywords();

    /**
     * @brief Compute a list of pairs (word -> color) to be used for coloration by the REPL
     * @return std::vector<std::pair<std::string, replxx::Replxx::Color>>
     */
    std::vector<std::pair<std::string, replxx::Replxx::Color>> getColorPerKeyword();

    replxx::Replxx::completions_t hookCompletion(const std::vector<std::string>& words, const std::string& context, int& length);

    void hookColor(const std::vector<std::pair<std::string, replxx::Replxx::Color>>& words_colors, const std::string& context, replxx::Replxx::colors_t& colors);

    replxx::Replxx::hints_t hookHint(const std::vector<std::string>& words, const std::string& context, int& length, replxx::Replxx::Color& color);
}

#endif
