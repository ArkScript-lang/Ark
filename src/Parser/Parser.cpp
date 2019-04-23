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

            if (m_ast.nodeType() == NodeType::List)
            {
                int i = 0;
                for (auto& node: m_ast.list())
                    std::cout << (i++) << ": " << node;
            }
            else
                std::cout << "not list" << std::endl << m_ast;
        }

        Node Parser::compile(std::list<std::string>& tokens)
        {
            const std::string token = tokens.front();
            tokens.pop_front();

            if (token == "(")
            {
                Node n(NodeType::List);
                while (tokens.front() != ")")
                    n.push_back(compile(tokens));
                tokens.pop_front();
                return n;
            }
            else
                return atom(token);
        }

        Node Parser::atom(const std::string& token)
        {
            if (Ark::Utils::isInteger(token))
                return Node(NodeType::Number, std::atoi(token.c_str()));
            if (Ark::Utils::isFloat(token))
                return Node(NodeType::Number, (float) std::atof(token.c_str()));
            return Node(NodeType::Symbol, token);
        }
    }
}