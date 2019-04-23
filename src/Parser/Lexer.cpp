#include <Ark/Parser/Lexer.hpp>

#include <Ark/Utils.hpp>
#include <Ark/Log.hpp>

namespace Ark
{
    namespace Parser
    {
        Lexer::Lexer(bool debug) :
            m_debug(debug)
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
                            m_tokens.push_back({ m[0], line, character });
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
                    Ark::logger.error("[Lexer] Tokenizing error at " + Ark::Utils::toString(line) + ":" + Ark::Utils::toString(character - 1));
                    break;
                }
            }

            if (m_debug)
            {
                Ark::logger.info("(Lexer) Tokens:");

                line = 0;
                for (auto&& token : m_tokens)
                {
                    if (token.line != line)
                    {
                        line = token.line;
                        if (line < 10)               std::cout << "   " << line;
                        else if (10 <= line < 100)   std::cout << "  " << line;
                        else if (100 <= line < 1000) std::cout << " " << line;
                        else                         std::cout << line;
                        std::cout << " | " << token.token << "\n";
                    }
                    else
                        std::cout << "     | " << token.token << "\n";
                }
                // flush
                std::cout << std::endl;
            }
        }

        bool Lexer::check()
        {
            // checks if code is correctly written
            std::size_t lparen = 0, rparen = 0;
            std::size_t nested_lparen = 0;
            std::size_t i = 0;

            while (true)
            {
                auto t = m_tokens[i].token;

                if (t == "(" || t == "[" || t == "{")
                {
                    lparen++;
                    nested_lparen++;
                }
                if (t == ")" || t == "]" || t == "}")
                {
                    rparen++;
                    if (nested_lparen > 0)
                        nested_lparen--;
                    else
                    {
                        Ark::logger.error(std::string("[Lexer] Found a ')' not matching any '('. At {0}:{1}"), m_tokens[i].line, m_tokens[i].col);
                        return false;
                    }
                }

                // go to next token
                if (i < m_tokens.size() - 1)
                    i++;
                else
                    break;
            }

            if (lparen != rparen)
            {
                Ark::logger.error("[Lexer] Found " + Ark::Utils::toString(lparen) + std::string(" '(' for ") + Ark::Utils::toString(rparen) + std::string(" ')'"));
                return false;
            }

            return true;
        }

        const std::vector<Token>& Lexer::tokens()
        {
            return m_tokens;
        }
    }
}
