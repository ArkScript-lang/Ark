#ifndef ark_parser
#define ark_parser

#include <string>
#include <list>
#include <iostream>

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
            const Node& ast();

            friend std::ostream& operator<<(std::ostream& os, const Parser& P);

        private:
            Lexer m_lexer;
            Node m_ast;

            Node compile(std::list<std::string>& tokens);
            Node atom(const std::string& token);
            const Node& const_ast() const;
        };
    }
}

#endif  // ark_parser
