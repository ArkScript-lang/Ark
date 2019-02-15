#include <Ark/Parser/Parser.hpp>

#include <Ark/Log.hpp>

namespace Ark
{
    namespace Parser
    {
        Parser::Parser()
        {}

        Parser::~Parser()
        {}

        void Parser::feed(const std::string& code)
        {
            m_lexer.feed(code);
            if (!m_lexer.check())
            {
                Ark::Log::error("[Parser] Can not continue, lexer has errors for you");
                return;
            }

            // create program and raise error if it can't
            for (auto& t: m_lexer.tokens())
            {
                std::cout << t << std::endl;
            }
        }
    }
}