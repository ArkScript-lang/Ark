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
        using namespace Ark::Lang;
    
        class Parser
        {
        public:
            Parser();
            ~Parser();

            void feed(const std::string& code);
            bool check();
            const Node& ast() const;

            friend std::ostream& operator<<(std::ostream& os, const Parser& P);

        private:
            Lexer m_lexer;
            Node m_ast;

            Node compile(std::list<std::string>& tokens);
            Node atom(const std::string& token);
            bool _check(const Node& ast);
        };
    }
}

#endif  // ark_parser
