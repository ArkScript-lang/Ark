/**
 * @file Util.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief replxx utilities
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef REPL_REPLXX_UTIL_HPP
#define REPL_REPLXX_UTIL_HPP

#include <vector>
#include <utility>
#include <string>

#include <replxx.hxx>

using Replxx = replxx::Replxx;

int utf8str_codepoint_len(char const *s, int utf8len);
int context_len(char const *prefix);
Replxx::completions_t hook_completion(std::string const& context, int& contextLen, std::vector<std::string> const& user_data);
void hook_color(std::string const& str, Replxx::colors_t& colors, std::vector<std::pair<std::string, Replxx::Color>> const& user_data);
Replxx::hints_t hook_hint(std::string const& context, int& contextLen, Replxx::Color& color, std::vector<std::string> const& examples);

#endif
