#include <Ark/Parser/Parser.hpp>

#include <optional>
#include <algorithm>

#include <Ark/Log.hpp>
#include <Ark/Utils.hpp>

namespace Ark
{
    namespace Parser
    {
        Parser::Parser(bool debug) :
            m_debug(debug),
            m_lexer(debug),
            m_file("FILE")
        {}

        void Parser::feed(const std::string& code, const std::string& filename)
        {
            // not the default value
            if (filename != "FILE")
                m_file = Ark::Utils::canonicalRelPath(filename);

            m_lexer.feed(code);
            if (!m_lexer.check())
            {
                Ark::logger.error("[Parser] Can not continue, lexer has errors for you");
                exit(1);
            }

            // apply syntactic sugar
            std::vector<Token> t = m_lexer.tokens();
            sugar(t);
            // create program and raise error if it can't
            std::list<Token> tokens(t.begin(), t.end());
            m_ast = compile(tokens);
            // include files if needed
            checkForInclude(m_ast);
            // expend macros
            checkForQuote(m_ast);

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

        void Parser::sugar(std::vector<Token>& tokens)
        {
            std::size_t i = 0;
            while (true)
            {
                std::size_t line = tokens[i].line;
                std::size_t col = tokens[i].col;
                
                if (tokens[i].token == "{")
                {
                    tokens[i] = Token("(", line, col);
                    tokens.insert(tokens.begin() + i + 1, Token("begin", line, col));
                }
                else if (tokens[i].token == "}" || tokens[i].token == "]")
                    tokens[i] = Token(")", line, col);
                else if (tokens[i].token == "[")
                {
                    tokens[i] = Token("(", line, col);
                    tokens.insert(tokens.begin() + i + 1, Token("list", line, col));
                }

                ++i;

                if (i == tokens.size())
                    break;
            }
        }

        Node Parser::compile(std::list<Token>& tokens)
        {
            const Token t = tokens.front();
            const std::string token = t.token;
            tokens.pop_front();

            if (token == "(")
            {
                Node n(NodeType::List);
                n.setPos(t.line, t.col);
                while (tokens.front().token != ")")
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
                auto n = Node(NodeType::Number, Ark::BigNum(token.token));
                n.setPos(token.line, token.col);
                return n;
            }
            if (Ark::Utils::isFloat(token.token))
            {
                std::string content = token.token;
                auto it = content.find_first_of('.');
                std::size_t decimals = std::distance(content.begin() + it, content.end());
                content.erase(content.begin() + it);
                content += "/1";
                for (std::size_t _i=0; ++_i != decimals; content += "0");

                auto n = Node(NodeType::Number, Ark::BigNum(content));
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
            else if (token.token == "set")
                kw = Keyword::Set;
            else if (token.token == "let")
                kw = Keyword::Let;
            else if (token.token == "fun")
                kw = Keyword::Fun;
            else if (token.token == "while")
                kw = Keyword::While;
            else if (token.token == "begin")
                kw = Keyword::Begin;
            else if (token.token == "import")
                kw = Keyword::Import;
            else if (token.token == "quote")
                kw = Keyword::Quote;
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

        bool Parser::checkForInclude(Node& n)
        {
            if (n.nodeType() == NodeType::Keyword)
            {
                if (n.keyword() == Keyword::Import)
                    return true;
            }
            else if (n.nodeType() == NodeType::List)
            {
                for (std::size_t i=0; i < n.list().size(); ++i)
                {
                    if (checkForInclude(n.list()[i]))
                    {
                        if (m_debug)
                            Ark::logger.info("Import found in file:", m_file);
                        
                        std::string file;
                        if (n.const_list()[1].nodeType() == NodeType::String)
                                file = n.const_list()[1].getStringVal();
                        else
                            throw Ark::TypeError("Arguments of import must be Strings");

                        namespace fs = std::filesystem;
                        using namespace std::string_literals;

                        std::string ext = fs::path(file).extension().string();
                        std::string path = Ark::Utils::getDirectoryFromPath(m_file) + "/" + file;
                        
                        if (m_debug)
                            Ark::logger.data(path);
                        
                        // check if we are not loading a plugin
                        if (ext == ".ark")
                        {
                            // replace content with a begin block
                            n.list().clear();
                            n.list().emplace_back(NodeType::Keyword, Keyword::Begin);

                            std::string f = fs::relative(fs::path(path), fs::path(m_parent_include.size() ? m_parent_include.back() : "").root_path()).string();
                            if (m_debug)
                                Ark::logger.info("Importing:", file, "; relative path:", f);

                            if (std::find(m_parent_include.begin(), m_parent_include.end(), f) == m_parent_include.end())
                            {
                                Parser p(m_debug);

                                for (auto&& pi : m_parent_include)
                                    p.m_parent_include.push_back(pi);
                                p.m_parent_include.push_back(m_file);

                                // search in the files of the user first
                                if (Ark::Utils::fileExists(path))
                                    p.feed(Ark::Utils::readFile(path), path);
                                else
                                {
                                    std::string libpath = std::string(ARK_STD) + "/" + Ark::Utils::getFilenameFromPath(path);
                                    if (Ark::Utils::fileExists(libpath))
                                        p.feed(Ark::Utils::readFile(libpath), libpath);
                                    else
                                    {
                                        Ark::logger.error("Couldn't find file", file);
                                        exit(1);
                                    }
                                }

                                if (p.check())
                                    n.list().push_back(p.ast());
                                else
                                    exit(1);
                            }
                            else
                            {
                                Ark::logger.error("Cyclic inclusion issue: file {0} is trying to include {1} which was already included"s, m_file, path);
                                exit(1);
                            }
                        }
                    }
                }
            }
            
            return false;
        }

        void Parser::checkForQuote(Node& n)
        {
            if (n.nodeType() == NodeType::List)
            {
                std::size_t i = 0;
                Nodes::iterator it = n.list().begin();

                while (true)
                {
                    if (it == n.list().end())
                        break;

                    if (it->nodeType() == NodeType::Symbol && it->getStringVal() == "`")
                    {
                        Node temp(NodeType::List);

                        temp.push_back(Node(NodeType::Keyword, Keyword::Quote));
                        while (true)
                        {
                            Nodes::iterator it2 = n.list().begin() + 1 + i;
                            if (it2 == n.list().end())
                                break;
                            
                            checkForQuote(*it2);
                            temp.push_back(*it2);
                            n.list().erase(it2);
                        }

                        *it = temp;
                        it = n.list().begin();
                    }
                    else if (it->nodeType() == NodeType::List)
                        checkForQuote(*it);

                    ++i;
                    ++it;
                }
            }
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
                                        Ark::logger.error("[Parser] function needs a symbols' list as 1st argument, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                        return false;
                                    }
                                    
                                    for (Node::Iterator it2=p[1].const_list().begin(); it2 != p[1].const_list().end(); ++it2)
                                    {
                                        if (it2->nodeType() != NodeType::Symbol)
                                        {
                                            Ark::logger.error("[Parser] found a non-symbol in function arguments' list, at {0}:{1} in {2}"s, it2->line(), it2->col(), *it2);
                                            return false;
                                        }
                                    }
                                    
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] function body is ill-formed, at {0}:{1} in {2}"s, p[2].line(), p[2].col(), p);
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to create a function: the arguments' list and the function body, at {0}:{1} in {2}"s, n.line(), n.col(), n);
                                return false;
                            }
                            
                            case Keyword::Let:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::logger.error("[Parser] need a symbol to name a variable, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] value needed to define a variable is ill-formed, at {0}:{1} in {2}"s, p[2].line(), p[2].col(), p);
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to create a variable: the variable name to create and the value, at {0}:{1}, got {2} in {3}"s, n.line(), n.col(), p.size(), p);
                                return false;
                            }
                            
                            case Keyword::Set:
                            {
                                if (p.size() == 3)
                                {
                                    if (p[1].nodeType() != NodeType::Symbol)
                                    {
                                        Ark::logger.error("[Parser] need a symbol to find the variable to set, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] value given to variable is ill-formed, at {0}:{1} in {2}"s, p[2].line(), p[2].col(), p);
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to set a variable: the variable name and the new value, at {0}:{1} in {2}"s, n.line(), n.col(), p);
                                return false;
                            }
                            
                            case Keyword::If:
                            {
                                if (p.size() == 4)
                                {
                                    if (!_check(p[1]))
                                    {
                                        Ark::logger.error("[Parser] if condition is ill-formed, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] if-condition: then part is ill-formed, at {0}:{1} in {2}"s, p[2].line(), p[2].col(), p);
                                        return false;
                                    }
                                    if (!_check(p[3]))
                                    {
                                        Ark::logger.error("[Parser] if-condition: else part is ill-formed, at {0}:{1} in {2}"s, p[3].line(), p[3].col(), p);
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 3 nodes to create an if-condition: the condition, the then part, and the else part, at {0}:{1} in {2}"s, n.line(), n.col(), p);
                                return false;
                            }
                            
                            case Keyword::While:
                            {
                                if (p.size() == 3)
                                {
                                    if (!_check(p[1]))
                                    {
                                        Ark::logger.error("[Parser] while condition is ill-formed, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                        return false;
                                    }
                                    if (!_check(p[2]))
                                    {
                                        Ark::logger.error("[Parser] while body is ill-formed, at {0}:{1} in {2}"s, p[2].line(), p[2].col(), p);
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need 2 nodes to create a while loop: the condition and the body, at {0}:{1} in {2}"s, n.line(), n.col(), p);
                                return false;
                            }
                            
                            case Keyword::Begin:
                            {
                                for (Node::Iterator it2=p.begin()+1; it2 != p.end(); ++it2)
                                {
                                    if (!_check(*it2))
                                    {
                                        Ark::logger.error("[Parser] begin node contains an ill-formed node, at {0}:{1} in {2}"s, it2->line(), it2->col(), *it2);
                                        return false;
                                    }
                                }
                                return true;
                            }

                            case Keyword::Import:
                            {
                                if (p.size() == 2)
                                {
                                    if (p[1].nodeType() != NodeType::String)
                                    {
                                        Ark::logger.error("[Parser] import needs String, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                        return false;
                                    }
                                    return true;
                                }
                                Ark::logger.error("[Parser] need at least 1 plugin name");
                                return false;
                            }

                            case Keyword::Quote:
                            {
                                if (p.size() == 2)
                                    return true;
                                Ark::logger.error("[Parser] quote needs a single argument, at {0}:{1} in {2}"s, p[1].line(), p[1].col(), p);
                                return false;
                            }
                            
                            default:
                                Ark::logger.error("[Parser] unknown keyword type, at {0}:{1} in {2}"s, n.line(), n.col(), p);
                                return false;
                        }
                        break;
                    }
                    
                    case NodeType::Symbol:
                    {
                        std::string s = n.getStringVal();
                        bool b = true;
                        // 2 or more arguments
                        if (s == "+" || s == "-" || s == "/" || s == "*" || s == "and" || s == "or")
                            b = p.size() > 2;
                        // exactly 2 arguments
                        else if (s == "<" || s == ">" || s == "<=" || s == ">=" || s == "!=" || s == "=" || s == "assert" || s == "@")
                            b = p.size() == 3;
                        // exactly 1 argument
                        else if (s == "len" || s == "empty?" || s == "firstof" || s == "tailof" || s == "headof" || s == "nil?" || s == "toNumber" || s == "toString")
                            b = p.size() == 2;
                        // at least 1 argument
                        else if (s == "append" || s == "cons" || s == "print")
                            b = p.size() > 1;
                        // 0 or 1 argument
                        else if (s == "input")
                            b = p.size() <= 2;
                        // at least 0 arguments
                        else if (s == "list")
                            b = p.size() >= 1;
                        if (!b)
                            Ark::logger.error("[Parser] builtin function '" + s + "' has the wrong number of arguments, at {0}:{1} in {2}"s, n.line(), n.col(), p);
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
                            Ark::logger.error("[Parser] ill-formed node list, at {0}:{1} in {2}"s, n.line(), n.col(), p);
                            return false;
                        }
                        return true;
                    }
                    
                    default:
                        Ark::logger.error("[Parser] node type unknow, at {0}:{1} in {2}"s, n.line(), n.col(), p);
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
