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
        const internal::Node& ast() const;

        friend std::ostream& operator<<(std::ostream& os, const Parser& P);

    private:
        bool m_debug;
        internal::Lexer m_lexer;
        internal::Node m_ast;

        std::string m_file;
        std::vector<std::string> m_parent_include;

        void sugar(std::vector<internal::Token>& tokens);
        internal::Node parse(std::list<internal::Token>& tokens, bool authorize_capture=false);
        internal::Token nextToken(std::list<internal::Token>& tokens);
        internal::Node atom(const internal::Token& token);

        bool checkForInclude(internal::Node& n);

        inline void throwParseError(const std::string& message, internal::Token token)
        {
            throw std::runtime_error("ParseError: " + message + "\nAt " +
                Ark::Utils::toString(token.line) + ":" + Ark::Utils::toString(token.col) +
                " `" + token.token + "' (" + internal::tokentype_string[static_cast<unsigned>(token.type)] + ")"
            );
        }
    };
}

#endif  // ark_parser
