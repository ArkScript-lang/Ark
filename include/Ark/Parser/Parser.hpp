#ifndef ark_parser
#define ark_parser

#include <string>
#include <list>

#include <Ark/Parser/Lexer.hpp>
#include <Ark/Parser/Node.hpp>

namespace Ark
{
    namespace Parser
    {
        class Parser
        {
        public:
            Parser();
            ~Parser();

            void feed(const std::string& code);

        private:
            Lexer m_lexer;
            Node m_ast;

            Node compile(std::list<std::string>& tokens);
            Node atom(const std::string& token);
        };
    }
}

#endif  // ark_parser