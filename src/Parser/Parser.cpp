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
        
        bool Parser::check()
        {
            return _check(m_ast);
        }

        const Node& Parser::ast() const
        {
            return m_ast;
        }

        Node Parser::compile(std::list<std::string>& tokens)
        {
            const std::string token = tokens.front();
            tokens.pop_front();

            if (token == "(" || token == "[" || token == "{")
            {
                Node n(NodeType::List);
                while (tokens.front() != ")" && tokens.front() != "]" && tokens.front() != "}")
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
            if (token[0] == '"')
            {
                std::string str = token;
                str.erase(0, 1);
                str.erase(token.size() - 2, 1);
                return Node(NodeType::String, str);
            }
            
            if (token == "if")
                return Node(NodeType::Keyword, Keyword::If);
            if (token == "set")
                return Node(NodeType::Keyword, Keyword::Set);
            if (token == "def")
                return Node(NodeType::Keyword, Keyword::Def);
            if (token == "fun")
                return Node(NodeType::Keyword, Keyword::Fun);
            if (token == "while")
                return Node(NodeType::Keyword, Keyword::While);
            if (token == "begin")
                return Node(NodeType::Keyword, Keyword::Begin);
            
            return Node(NodeType::Symbol, token);
        }
        
        bool Parser::_check(const Node& ast)
        {
            if (ast.nodeType() == NodeType::List)
            {
                const Nodes& p = ast.const_list();
                if (p.size() == 0)
                    return true;
                
                Node n = p[0];
                
                switch (n.nodeType())
                {
                    case NodeType::Keyword:
                    {
                        switch (n.keyword())
                        {
                            case Keyword::Fun:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::List)
                                    {
                                        Ark::Log::error("[Parser] error: function needs a symbols' list as 1st argument");
                                        return false;
                                    }
                                    
                                    for (Node::Iterator it2=p[1].const_list().begin(); it2 != p[1].const_list().end(); ++it2)
                                    {
                                        if (it2->nodeType() != NodeType::Symbol)
                                        {
                                            Ark::Log::error("[Parser] error: found a non-symbol in function arguments' list");
                                            return false;
                                        }
                                    }
                                    
                                    if (!_check(p[2]))
                                    {
                                        Ark::Log::error("[Parser] error: function body is ill-formed");
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::Log::error("[Parser] error: need 2 nodes to create a function: the arguments' list and the function body");
                                return false;
                            }
                            
                            case Keyword::Def:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::Log::error("[Parser] error: need a symbol to name a variable");
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::Log::error("[Parser] error: value needed to define a variable is ill-formed");
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::Log::error("[Parser] error: need 2 nodes to create a variable: the variable name to create and the value");
                                return false;
                            }
                            
                            case Keyword::Set:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::Log::error("[Parser] error: need a symbol to find the variable to set");
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::Log::error("[Parser] error: value given to variable is ill-formed");
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::Log::error("[Parser] error: need 2 nodes to set a variable: the variable name and the new value");
                                return false;
                            }
                            
                            case Keyword::If:
                            {
                                if (p.size() == 4)
                                {
                                    if (!_check(p[1]))
                                    {
                                        Ark::Log::error("[Parser] error: if condition is ill-formed");
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::Log::error("[Parser] error: if-condition: then part is ill-formed");
                                        return false;
                                    }
                                    if (!_check(p[3]))
                                    {
                                        Ark::Log::error("[Parser] error: if-condition: else part is ill-formed");
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::Log::error("[Parser] error: need 3 nodes to create an if-condition: the condition, the then part, and the else part");
                                return false;
                            }
                            
                            case Keyword::While:
                            {
                                if (p.size() == 3)
                                {
                                    if (!_check(p[1]))
                                    {
                                        Ark::Log::error("[Parser] error: while condition is ill-formed");
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::Log::error("[Parser] error: while body is ill-formed");
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::Log::error("[Parser] error: need 2 nodes to create a while loop: the condition and the body");
                                return false;
                            }
                            
                            case Keyword::Begin:
                            {
                                for (Node::Iterator it2=p.begin()+1; it2 != p.end(); ++it2)
                                {
                                    if (!_check(*it2))
                                    {
                                        Ark::Log::error("[Parser] error: begin node contains an ill-formed node");
                                        return false;
                                    }
                                }
                                return true;
                            }
                            
                            default:
                                Ark::Log::error("[Parser] error: unknown keyword type");
                                return false;
                        }
                        break;
                    }
                    
                    case NodeType::Symbol:
                    {
                        std::string s = n.getStringVal();
                        bool b = true;
                        if (s == "+" || s == "-" || s == "/" || s == "*")
                            b = p.size() > 2;
                        else if (s == "<" || s == ">" || s == "<=" || s == ">=" || s == "!=" || s == "=" || s == "assert")
                            b = p.size() == 3;
                        else if (s == "len" || s == "empty" || s == "car" || s == "cdr" || s == "nil?")
                            b = p.size() == 2;
                        else if (s == "append" || s == "cons" || s == "list" || s == "print")
                            b = p.size() > 1;
                        if (!b)
                            Ark::Log::error("[Parser] error: builtin function '" + s + "' has the wrong number of arguments");
                        return b;
                    }
                    
                    case NodeType::String:
                    case NodeType::Number:
                    {
                        return true;
                    }
                    
                    case NodeType::List:
                    {
                        if (!_check(n))
                        {
                            Ark::Log::error("[Parser] error: ill-formed node list");
                            return false;
                        }
                        return true;
                    }
                    
                    default:
                        Ark::Log::error("[Parser] error: node type unknow");
                        return false;
                }
            }
            else
            {
                // keyword alone shouldn't work
                if (ast.nodeType() != NodeType::Keyword)
                    return true;
                Ark::Log::error("[Parser] error: keyword node can not be alone in the dark");
                return false;
            }
        }

        std::ostream& operator<<(std::ostream& os, const Parser& P)
        {
            os << "AST" << std::endl;
            if (P.ast().nodeType() == NodeType::List)
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
