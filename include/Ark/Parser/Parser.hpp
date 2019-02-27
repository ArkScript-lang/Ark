#ifndef ark_parser
#define ark_parser

#include <string>
#include <list>
#include <iostream>

#include <Ark/Parser/Lexer.hpp>
#include <Ark/Lang/Node.hpp>

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
            const Ark::Lang::Node& ast() const;

            friend std::ostream& operator<<(std::ostream& os, const Parser& P);

        private:
            Lexer m_lexer;
            Ark::Lang::Node m_ast;

            Ark::Lang::Node compile(std::list<std::string>& tokens);
            Ark::Lang::Node atom(const std::string& token);
        };
    }
}

#endif  // ark_parser
