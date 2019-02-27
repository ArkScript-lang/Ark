#include <Ark/Parser/Parser.hpp>

#include <Ark/Log.hpp>
#include <Ark/Utils.hpp>

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
            std::list<std::string> tokens(m_lexer.tokens().begin(), m_lexer.tokens().end());
            m_ast = compile(tokens);
        }

        const Ark::Lang::Node& Parser::ast() const
        {
            return m_ast;
        }

        Ark::Lang::Node Parser::compile(std::list<std::string>& tokens)
        {
            const std::string token = tokens.front();
            tokens.pop_front();

            if (token == "(")
            {
                Ark::Lang::Node n(Ark::Lang::NodeType::List);
                while (tokens.front() != ")")
                    n.push_back(compile(tokens));
                tokens.pop_front();
                return n;
            }
            else
                return atom(token);
        }

        Ark::Lang::Node Parser::atom(const std::string& token)
        {
            if (Ark::Utils::isInteger(token))
                return Ark::Lang::Node(Ark::Lang::NodeType::Number, std::atoi(token.c_str()));
            if (Ark::Utils::isFloat(token))
                return Ark::Lang::Node(Ark::Lang::NodeType::Number, (float) std::atof(token.c_str()));
            if (token[0] == '"')
            {
                std::string str = token;
                str.erase(0, 1);
                str.erase(token.size() - 2, 1);
                return Ark::Lang::Node(Ark::Lang::NodeType::String, str);
            }
            return Ark::Lang::Node(Ark::Lang::NodeType::Symbol, token);
        }

        std::ostream& operator<<(std::ostream& os, const Parser& P)
        {
            os << "AST" << std::endl;
            if (P.ast().nodeType() == Ark::Lang::NodeType::List)
            {
                int i = 0;
                for (const auto& node: P.ast().const_list())
                    std::cout << (i++) << ": " << node << std::endl;
            }
            else
                os << "Single item" << std::endl << P.m_ast << std::endl;
            return os;
        }
    }
}
