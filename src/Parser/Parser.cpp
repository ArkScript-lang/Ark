#include <Ark/Parser/Parser.hpp>

#include <optional>

#include <Ark/Log.hpp>
#include <Ark/Utils.hpp>

namespace Ark
{
    namespace Parser
    {
        Parser::Parser(bool debug) :
            m_debug(debug),
            m_lexer(debug)
        {}

        Parser::~Parser()
        {}

        void Parser::feed(const std::string& code)
        {
            m_lexer.feed(code);
            if (!m_lexer.check())
            {
                Ark::logger.error("[Parser] Can not continue, lexer has errors for you");
                exit(1);
            }

            // create program and raise error if it can't
            std::list<Token> tokens(m_lexer.tokens().begin(), m_lexer.tokens().end());
            m_ast = compile(tokens);

            if (m_debug)
            {
                Ark::logger.info("(Parser) AST:");
                std::cout << m_ast << std::endl << std::endl;
            }
        }
        
        bool Parser::check()
        {
            return _check(m_ast);
        }

        const Node& Parser::ast() const
        {
            return m_ast;
        }

        Node Parser::compile(std::list<Token>& tokens)
        {
            const Token t = tokens.front();
            const std::string token = t.token;
            tokens.pop_front();

            if (token == "(" || token == "[" || token == "{")
            {
                Node n(NodeType::List);
                n.setPos(t.line, t.col);
                while (tokens.front().token != ")" && tokens.front().token != "]" && tokens.front().token != "}")
                    n.push_back(compile(tokens));
                tokens.pop_front();
                return n;
            }
            else
                return atom(t);
        }

        Node Parser::atom(const Token& token)
        {
            if (Ark::Utils::isInteger(token.token) || Ark::Utils::isFraction(token.token))
            {
                auto n = Node(NodeType::Number, Ark::BigNum(token.token.c_str()));
                n.setPos(token.line, token.col);
                return n;
            }
            if (token.token[0] == '"')
            {
                std::string str = token.token;
                str.erase(0, 1);
                str.erase(token.token.size() - 2, 1);

                auto n = Node(NodeType::String, str);
                n.setPos(token.line, token.col);
                return n;
            }
            
            std::optional<Keyword> kw;
            if (token.token == "if")
                kw = Keyword::If;
            if (token.token == "set")
                kw = Keyword::Set;
            if (token.token == "let")
                kw = Keyword::Def;
            if (token.token == "fun")
                kw = Keyword::Fun;
            if (token.token == "while")
                kw = Keyword::While;
            if (token.token == "begin")
                kw = Keyword::Begin;
            if (token.token == "hastype")
                kw = Keyword::HasType;
            if (kw)
            {
                auto n = Node(NodeType::Keyword, kw.value());
                n.setPos(token.line, token.col);
                return n;
            }
            
            auto n = Node(NodeType::Symbol, token.token);
            n.setPos(token.line, token.col);
            return n;
        }
        
        bool Parser::_check(const Node& ast)
        {
            using namespace std::string_literals;

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
                                        Ark::logger.error("[Parser] function needs a symbols' list as 1st argument, at {0}:{1}"s, p[1].line(), p[1].col());
                                        return false;
                                    }
                                    
                                    for (Node::Iterator it2=p[1].const_list().begin(); it2 != p[1].const_list().end(); ++it2)
                                    {
                                        if (it2->nodeType() != NodeType::Symbol)
                                        {
                                            Ark::logger.error("[Parser] found a non-symbol in function arguments' list, at {0}:{1}"s, it2->line(), it2->col());
                                            return false;
                                        }
                                    }
                                    
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] function body is ill-formed, at {0}:{1}"s, p[2].line(), p[2].col());
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to create a function: the arguments' list and the function body, at {0}:{1}"s, n.line(), n.col());
                                return false;
                            }
                            
                            case Keyword::Def:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::logger.error("[Parser] need a symbol to name a variable, at {0}:{1}"s, p[1].line(), p[1].col());
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] value needed to define a variable is ill-formed, at {0}:{1}"s, p[2].line(), p[2].col());
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to create a variable: the variable name to create and the value, at {0}:{1}, got {2}"s, n.line(), n.col(), p.size());
                                return false;
                            }
                            
                            case Keyword::Set:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::logger.error("[Parser] need a symbol to find the variable to set, at {0}:{1}"s, p[1].line(), p[1].col());
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] value given to variable is ill-formed, at {0}:{1}"s, p[2].line(), p[2].col());
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to set a variable: the variable name and the new value, at {0}:{1}"s, n.line(), n.col());
                                return false;
                            }
                            
                            case Keyword::If:
                            {
                                if (p.size() == 4)
                                {
                                    if (!_check(p[1]))
                                    {
                                        Ark::logger.error("[Parser] if condition is ill-formed, at {0}:{1}"s, p[1].line(), p[1].col());
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] if-condition: then part is ill-formed, at {0}:{1}"s, p[2].line(), p[2].col());
                                        return false;
                                    }
                                    if (!_check(p[3]))
                                    {
                                        Ark::logger.error("[Parser] if-condition: else part is ill-formed, at {0}:{1}"s, p[3].line(), p[3].col());
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 3 nodes to create an if-condition: the condition, the then part, and the else part, at {0}:{1}"s, n.line(), n.col());
                                return false;
                            }
                            
                            case Keyword::While:
                            {
                                if (p.size() == 3)
                                {
                                    if (!_check(p[1]))
                                    {
                                        Ark::logger.error("[Parser] while condition is ill-formed, at {0}:{1}"s, p[1].line(), p[1].col());
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] while body is ill-formed, at {0}:{1}"s, p[2].line(), p[2].col());
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to create a while loop: the condition and the body, at {0}:{1}"s, n.line(), n.col());
                                return false;
                            }

                            case Keyword::HasType:
                            {
                                if (p.size() == 4)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::logger.error("[Parser] need a symbol to define a has-type rule, at {0}:{1}"s, p[1].line(), p[1].col());
                                        return false;
                                    }
                                    if (p[2].nodeType() != NodeType::List)
                                    {
                                        Ark::logger.error("[Parser] need a list to define the arguments of a function in a has-type rule, at {0}:{1}"s, p[2].line(), p[2].col());
                                        return false;
                                    }
                                    for (Node::Iterator it2=p[2].const_list().begin(); it2 != p[2].const_list().end(); ++it2)
                                    {
                                        if (it2->nodeType() != NodeType::Symbol)
                                        {
                                            Ark::logger.error("[Parser] types for arguments in has-type rule should be symbols, at {0}:{1}"s, it2->line(), it2->col());
                                            return false;
                                        }
                                        if (it2->getStringVal() != "Number" &&
                                            it2->getStringVal() != "String" &&
                                            it2->getStringVal() != "Bool"   &&
                                            it2->getStringVal() != "List")
                                        {
                                            Ark::logger.error("[Parser] unknown type in has-type rule: '" + it2->getStringVal() + "', at {0}:{1}"s, it2->line(), it2->col());
                                            return false;
                                        }
                                    }
                                    if (p[3].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::logger.error("[Parser] need a symbol to define the return type of a function in a has-type rule, at {0}:{1}"s, p[3].line(), p[3].col());
                                        return false;
                                    }
                                    if (p[3].getStringVal() != "Number" &&
                                        p[3].getStringVal() != "String" &&
                                        p[3].getStringVal() != "Bool"   &&
                                        p[3].getStringVal() != "List")
                                    {
                                        Ark::logger.error("[Parser] unknown return type in has-type rule: '" + p[3].getStringVal() + "', at {0}:{1}"s, p[3].line(), p[3].col());
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 4 nodes to define a has-type rule, at {0}:{1}"s, n.line(), n.col());
                                return false;
                            }
                            
                            case Keyword::Begin:
                            {
                                for (Node::Iterator it2=p.begin()+1; it2 != p.end(); ++it2)
                                {
                                    if (!_check(*it2))
                                    {
                                        Ark::logger.error("[Parser] begin node contains an ill-formed node, at {0}:{1}"s, it2->line(), it2->col());
                                        return false;
                                    }
                                }
                                return true;
                            }
                            
                            default:
                                Ark::logger.error("[Parser] unknown keyword type, at {0}:{1}"s, n.line(), n.col());
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
                        else if (s == "len" || s == "empty?" || s == "car" || s == "cdr" || s == "nil?")
                            b = p.size() == 2;
                        else if (s == "append" || s == "cons" || s == "list" || s == "print")
                            b = p.size() > 1;
                        if (!b)
                            Ark::logger.error("[Parser] builtin function '" + s + "' has the wrong number of arguments, at {0}:{1}"s, n.line(), n.col());
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
                            Ark::logger.error("[Parser] ill-formed node list, at {0}:{1}"s, n.line(), n.col());
                            return false;
                        }
                        return true;
                    }
                    
                    default:
                        Ark::logger.error("[Parser] node type unknow, at {0}:{1}"s, n.line(), n.col());
                        return false;
                }
            }
            else
            {
                // keyword alone shouldn't work
                if (ast.nodeType() != NodeType::Keyword)
                    return true;
                Ark::logger.error("[Parser] keyword node can not be alone in the dark");
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
