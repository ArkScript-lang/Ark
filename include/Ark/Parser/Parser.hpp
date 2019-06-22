#ifndef ark_parser
#define ark_parser

#include <string>
#include <list>
#include <iostream>
#include <vector>

#include <Ark/Parser/Lexer.hpp>
#include <Ark/Parser/Node.hpp>

namespace Ark
{
    class Parser
    {
    public:
        Parser(bool debug=false);

        void feed(const std::string& code, const std::string& filename="FILE");
        bool check();
        const internal::Node& ast() const;

        friend std::ostream& operator<<(std::ostream& os, const Parser& P);

    private:
        bool m_debug;
        internal::Lexer m_lexer;
        internal::Node m_ast;

        std::string m_file;
        std::vector<std::string> m_parent_include;

        void sugar(std::vector<internal::Token>& tokens);
        internal::Node compile(std::list<internal::Token>& tokens);
        internal::Node atom(const internal::Token& token);
        bool checkForInclude(internal::Node& n);
        void checkForQuote(internal::Node& n);
        bool _check(const internal::Node& ast);
    };
}

#endif  // ark_parser
