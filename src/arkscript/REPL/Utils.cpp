#include <CLI/REPL/Utils.hpp>

#include <regex>
#include <algorithm>
#include <numeric>
#include <ranges>

#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Compiler/Common.hpp>

namespace Ark::internal
{
    std::vector<std::string> getAllKeywords()
    {
        std::vector<std::string> output;
        output.reserve(keywords.size() + Language::listInstructions.size() + Language::operators.size() + Builtins::builtins.size() + 2);

        std::ranges::transform(keywords, std::back_inserter(output), [](const auto& string_view) {
            return std::string(string_view);
        });
        std::ranges::transform(Language::listInstructions, std::back_inserter(output), [](const auto& string_view) {
            return std::string(string_view);
        });
        std::ranges::transform(Language::operators, std::back_inserter(output), [](const auto& string_view) {
            return std::string(string_view);
        });
        std::ranges::transform(std::ranges::views::keys(Builtins::builtins), std::back_inserter(output), [](const auto& string) {
            return string;
        });

        output.emplace_back(Language::And);
        output.emplace_back(Language::Or);
        output.emplace_back(Language::Apply);

        return output;
    }

    std::vector<std::pair<std::string, replxx::Replxx::Color>> getColorPerKeyword()
    {
        using namespace replxx;
        using K = std::string;
        using V = Replxx::Color;

        std::vector<std::pair<K, V>> output;
        output.reserve(keywords.size() + Language::listInstructions.size() + Language::operators.size() + Builtins::builtins.size() + 4);

        std::ranges::transform(keywords, std::back_inserter(output), [](const auto& string_view) {
            return std::make_pair(std::string(string_view), Replxx::Color::BRIGHTRED);
        });
        std::ranges::transform(Language::listInstructions, std::back_inserter(output), [](const auto& string_view) {
            return std::make_pair(std::string(string_view), Replxx::Color::GREEN);
        });
        std::ranges::transform(Language::operators, std::back_inserter(output), [](const auto& string_view) {
            auto safe_op = std::string(string_view);
            if (const auto it = safe_op.find_first_of(R"(-+=/*<>[]()?")"); it != std::string::npos)
                safe_op.insert(it, "\\");
            return std::make_pair(safe_op, Replxx::Color::BRIGHTBLUE);
        });
        std::ranges::transform(std::ranges::views::keys(Builtins::builtins), std::back_inserter(output), [](const auto& string) {
            return std::make_pair(string, Replxx::Color::GREEN);
        });

        output.emplace_back(Language::And, Replxx::Color::BRIGHTBLUE);
        output.emplace_back(Language::Or, Replxx::Color::BRIGHTBLUE);
        output.emplace_back(Language::Apply, Replxx::Color::BRIGHTBLUE);
        output.emplace_back("[\\-|+]?[0-9]+(\\.[0-9]+)?", Replxx::Color::YELLOW);
        output.emplace_back("\".*\"", Replxx::Color::MAGENTA);

        return output;
    }

    std::size_t codepointLength(const std::string& str)
    {
        return std::accumulate(
            str.begin(),
            str.end(),
            std::size_t { 0 },
            [](const std::size_t acc, const char c) {
                return acc + ((c & 0xc0) != 0x80);
            });
    }

    std::size_t contextLen(const std::string& prefix)
    {
        const std::string word_break = " \t\n\r\v\f=+*&^%$#@!,./?<>;`~'\"[]{}()\\|";
        std::size_t count = 0;

        for (const auto c : std::ranges::views::reverse(prefix))
        {
            if (word_break.find(c) != std::string::npos)
                break;
            ++count;
        }

        return count;
    }

    replxx::Replxx::completions_t hookCompletion(const std::vector<std::string>& words, const std::string& context, int& length)
    {
        replxx::Replxx::completions_t completions;
        std::size_t utf8_context_len = contextLen(context);
        std::size_t prefix_len = context.size() - utf8_context_len;

        if (prefix_len > 0 && context[prefix_len - 1] == '\\')
        {
            --prefix_len;
            ++utf8_context_len;
        }

        length = static_cast<int>(codepointLength(context.substr(prefix_len, utf8_context_len)));

        const std::string prefix = context.substr(prefix_len);
        for (const auto& e : words)
        {
            if (e.starts_with(prefix))
                completions.emplace_back(e.c_str());
        }

        return completions;
    }

    void hookColor(const std::vector<std::pair<std::string, replxx::Replxx::Color>>& words_colors, const std::string& context, replxx::Replxx::colors_t& colors)
    {
        // highlight matching regex sequences
        for (const auto& [regex, color] : words_colors)
        {
            std::size_t pos = 0;
            std::string str = context;
            std::smatch match;

            while (std::regex_search(str, match, std::regex(regex)))
            {
                std::string c = match[0];
                std::string prefix = match.prefix().str();
                const std::size_t len = codepointLength(c);

                pos += codepointLength(prefix);
                for (std::size_t i = 0; i < len; ++i)
                {
                    if (colors.at(pos + i) == replxx::Replxx::Color::DEFAULT)
                        colors.at(pos + i) = color;
                }

                pos += len;
                str = match.suffix();
            }
        }
    }

    replxx::Replxx::hints_t hookHint(const std::vector<std::string>& words, const std::string& context, int& length, replxx::Replxx::Color& color)
    {
        replxx::Replxx::hints_t hints;
        // only show hint if prefix is at least 'n' chars long
        // or if prefix begins with a specific character
        const std::size_t utf8_context_len = contextLen(context);
        const std::size_t prefix_len = context.size() - utf8_context_len;
        length = static_cast<int>(codepointLength(context.substr(prefix_len, utf8_context_len)));
        const std::string prefix = context.substr(prefix_len);

        if (prefix.size() >= 2 || (!prefix.empty() && prefix.at(0) == '.'))
        {
            for (const auto& e : words)
            {
                if (e.compare(0, prefix.size(), prefix) == 0)
                    hints.emplace_back(e.c_str());
            }
        }

        if (hints.size() == 1)
            color = replxx::Replxx::Color::GREEN;

        return hints;
    }
}
