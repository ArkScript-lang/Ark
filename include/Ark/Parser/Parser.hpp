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
            Parser(bool debug=false);
            ~Parser();

            void feed(const std::string& code);
            bool check();
            const Node& ast() const;

            friend std::ostream& operator<<(std::ostream& os, const Parser& P);

        private:
            bool m_debug;
            Lexer m_lexer;
            Node m_ast;

            Node compile(std::list<Token>& tokens);
            Node atom(const Token& token);
            bool _check(const Node& ast);
        };
    }
}

#endif  // ark_parser
