#include <Ark/REPL/replxx/Util.hpp>
#include <cstring>
#include <regex>

int utf8str_codepoint_len(char const *s, int utf8len)
{
    int codepointLen = 0;
    unsigned char m4 = 128 + 64 + 32 + 16;
    unsigned char m3 = 128 + 64 + 32;
    unsigned char m2 = 128 + 64;

    for(int i = 0; i < utf8len; ++i, ++codepointLen)
    {
        char c = s[i];

        if ((c & m4) == m4)
            i += 3;
        else if ((c & m3) == m3)
            i += 2;
        else if ((c & m2) == m2)
            i += 1;
    }

    return codepointLen;
}

int context_len(char const *prefix)
{
    char const wb[] = " \t\n\r\v\f-=+*&^%$#@!,./?<>;:`~'\"[]{}()\\|";
    int i = std::strlen(prefix) - 1;
    int cl = 0;

    while (i >= 0)
    {
        if (std::strchr(wb, prefix[i]) != NULL)
            break;

        ++cl;
        --i;
    }

    return cl;
}

Replxx::completions_t hook_completion(std::string const& context, int& contextLen, std::vector<std::string> const& examples)
{
    Replxx::completions_t completions;
    int utf8ContextLen = context_len(context.c_str());
    int prefixLen = context.length() - utf8ContextLen;

    if ((prefixLen > 0) && (context[prefixLen - 1] == '\\'))
    {
        --prefixLen;
        ++utf8ContextLen;
    }

    contextLen = utf8str_codepoint_len(context.c_str() + prefixLen, utf8ContextLen);

    std::string prefix = context.substr(prefixLen);
    for (auto const& e : examples)
    {
        if (e.compare(0, prefix.size(), prefix) == 0)
            completions.emplace_back(e.c_str());
    }

    return completions;
}

void hook_color(std::string const& context, Replxx::colors_t& colors, std::vector<std::pair<std::string, Replxx::Color>> const& regex_color)
{
    // highlight matching regex sequences
    for (auto const& e : regex_color)
    {
        std::size_t pos = 0;
        std::string str = context;
        std::smatch match;

        while (std::regex_search(str, match, std::regex(e.first)))
        {
            std::string c = match[0];
            std::string prefix = match.prefix().str();
            pos += utf8str_codepoint_len(prefix.c_str(), static_cast<int>(prefix.length()));
            int len = utf8str_codepoint_len(c.c_str(), static_cast<int>(c.length()));

            for (int i = 0; i < len; ++ i)
                colors.at(pos + i) = e.second;

            pos += len;
            str = match.suffix();
        }
    }
}

Replxx::hints_t hook_hint(std::string const& context, int& contextLen, Replxx::Color& color, std::vector<std::string> const& examples)
{
    Replxx::hints_t hints;
    // only show hint if prefix is at least 'n' chars long
    // or if prefix begins with a specific character
    int utf8ContextLen = context_len(context.c_str());
    int prefixLen = context.length() - utf8ContextLen;
    contextLen = utf8str_codepoint_len(context.c_str() + prefixLen, utf8ContextLen);
    std::string prefix = context.substr(prefixLen);

    if (prefix.size() >= 2 || (!prefix.empty() && prefix.at(0) == '.'))
    {
        for (auto const& e : examples)
        {
            if (e.compare(0, prefix.size(), prefix) == 0)
                hints.emplace_back(e.c_str());
        }
    }

    if (hints.size() == 1)
        color = Replxx::Color::GREEN;

    return hints;
}