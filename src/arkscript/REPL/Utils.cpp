#include <CLI/REPL/Utils.hpp>

#include <regex>
#include <algorithm>

namespace Ark::internal
{
    long countOpenEnclosures(const std::string& line, const char open, const char close)
    {
        return std::ranges::count(line, open) - std::ranges::count(line, close);
    }

    void trimWhitespace(std::string& line)
    {
        const std::size_t string_begin = line.find_first_not_of(" \t");
        if (std::string::npos != string_begin)
        {
            const std::size_t string_end = line.find_last_not_of(" \t");
            line = line.substr(string_begin, string_end - string_begin + 1);
        }
    }

    std::size_t codepointLength(const std::string& str)
    {
        std::size_t len = 0;
        for (const auto c : str)
            len += (c & 0xc0) != 0x80;
        return len;
    }

    std::size_t contextLen(const std::string& prefix)
    {
        const std::string word_break = " \t\n\r\v\f=+*&^%$#@!,./?<>;`~'\"[]{}()\\|";
        long i = static_cast<long>(prefix.size()) - 1;
        std::size_t count = 0;

        while (i >= 0)
        {
            if (word_break.find(prefix[static_cast<std::size_t>(i)]) != std::string::npos)
                break;

            ++count;
            --i;
        }

        return count;
    }

    replxx::Replxx::completions_t hookCompletion(const std::string& context, int& length)
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
        for (const auto& e : KeywordsDict)
        {
            if (e.starts_with(prefix) == 0)
                completions.emplace_back(e.c_str());
        }

        return completions;
    }

    void hookColor(const std::string& context, replxx::Replxx::colors_t& colors)
    {
        // highlight matching regex sequences
        for (const auto& [regex, color] : ColorsRegexDict)
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
                    colors.at(pos + i) = color;

                pos += len;
                str = match.suffix();
            }
        }
    }

    replxx::Replxx::hints_t hookHint(const std::string& context, int& length, replxx::Replxx::Color& color)
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
            for (const auto& e : KeywordsDict)
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
