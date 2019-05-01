#ifndef ark_parser
#define ark_parser

#include <string>
#include <list>
#include <iostream>
#include <vector>

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

            void feed(const std::string& code, const std::string& filename="FILE");
            bool check();
            const Node& ast() const;

            friend std::ostream& operator<<(std::ostream& os, const Parser& P);

        private:
            bool m_debug;
            Lexer m_lexer;
            Node m_ast;

            std::vector<std::string> m_include_files;
            std::string m_file;
            std::string m_parent_include;

            void sugar(std::vector<Token>& tokens);
            Node compile(std::list<Token>& tokens);
            Node atom(const Token& token);
            bool checkForInclude(Node& n);
            bool _check(const Node& ast);
        };
    }
}

#endif  // ark_parser
