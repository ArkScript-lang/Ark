#include <Ark/Parser/Lexer.hpp>

#include <Ark/Utils.hpp>
#include <Ark/Log.hpp>

namespace Ark
{
    namespace Parser
    {
        Lexer::Lexer()
        {}

        Lexer::~Lexer()
        {}

        void Lexer::feed(const std::string& code)
        {
            std::string s = code;
            std::size_t line = 1, character = 0;

            while (!s.empty())
            {
                bool ok = false;

                for (const auto& r : m_regexs)
                {
                    std::smatch m;
                    bool found = std::regex_search(s, m, r);
                    if (found)
                    {
                        std::string m0(m[0]);

                        // stripping blanks characters between instructions, and comments
                        if (std::string::npos != m0.find_first_not_of(" \t\v\r\n") && m0.substr(0, 1) != "'")
                            m_tokens.push_back(m[0]);
                        // line-char counter
                        if (std::string::npos != m0.find_first_of("\r\n"))
                        {
                            line++;
                            character = 0;
                        }
                        character += m[0].length();

                        s = std::regex_replace(s, r, std::string(""), std::regex_constants::format_first_only);
                        ok = true;
                        break;
                    }
                }
                if (!ok)
                {
                    Ark::Log::error("[Lexer] Tokenizing error at " + Ark::Utils::toString(line) + ":" + Ark::Utils::toString(character - 1));
                    break;
                }
            }

            #ifdef ARK_DEBUG
            for (auto& t: m_tokens)
            {
                std::cout << t << std::endl;
            }
            #endif  // ARK_DEBUG
        }

        bool Lexer::check()
        {
            // checks if code is correctly written
            std::size_t lparen = 0, rparen = 0;
            std::size_t nested_lparen = 0;
            std::size_t i = 0;

            while (true)
            {
                auto t = m_tokens[i];

                if (t == "(")
                {
                    lparen++;
                    nested_lparen++;
                }
                if (t == ")")
                {
                    rparen++;
                    if (nested_lparen > 0)
                        nested_lparen--;
                    else
                    {
                        Ark::Log::error("[Lexer] Found a ')' not matching any '('. Token number: " + Ark::Utils::toString(i));
                        return false;
                    }
                }
                
                if (t == "def")
                {
                    if (i < m_tokens.size() - 2)
                        i += 2;
                    else
                    {
                        Ark::Log::error("[Lexer] 'def' missing parameters. Token number: " + Ark::Utils::toString(i));
                        return false;
                    }
                    continue;
                }

                // go to next token
                if (i < m_tokens.size() - 1)
                    i++;
                else
                    break;
            }

            if (lparen != rparen)
            {
                Ark::Log::error("[Lexer] Found " + Ark::Utils::toString(lparen) + std::string(" '(' for ") + Ark::Utils::toString(rparen) + std::string(" ')'"));
                return false;
            }

            return true;
        }
    }
}