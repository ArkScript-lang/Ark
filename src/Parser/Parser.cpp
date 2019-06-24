#include <Ark/Parser/Parser.hpp>

#include <optional>
#include <algorithm>

#include <Ark/Log.hpp>
#include <Ark/Utils.hpp>

namespace Ark
{
    using namespace Ark::internal;

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

        // apply syntactic sugar
        std::vector<Token> t = m_lexer.tokens();
        sugar(t);
        // create program and raise error if it can't
        std::list<Token> tokens(t.begin(), t.end());
        m_ast = parse(tokens);
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
                tokens[i] = Token(TokenType::Grouping, "(", line, col);
                tokens.insert(tokens.begin() + i + 1, Token(TokenType::Keyword, "begin", line, col));
            }
            else if (tokens[i].token == "}" || tokens[i].token == "]")
                tokens[i] = Token(TokenType::Grouping, ")", line, col);
            else if (tokens[i].token == "[")
            {
                tokens[i] = Token(TokenType::Grouping, "(", line, col);
                tokens.insert(tokens.begin() + i + 1, Token(TokenType::Keyword, "list", line, col));
            }

            ++i;

            if (i == tokens.size())
                break;
        }
    }

    // sugar() was called before, so it's safe to assume we only have ( and )
    Node Parser::parse(std::list<Token>& tokens)
    {
        Token token = nextToken(tokens);

        if (token.token == "(")
        {
            // create a list node to host the block
            Node block(NodeType::List);
            block.setPos(token.line, token.col);

            // loop until we reach the end of the block
            while (tokens.front().token != ")")
            {
                block.push_back(atom(token));

                if (token.type == TokenType::Keyword)
                {
                    if (token.token == "if")
                    {
                        auto temp = tokens.front();
                        // parse condition
                        if (temp.type == TokenType::Grouping)
                            block.push_back(parse(tokens));
                        else if (temp.type == TokenType::Identifier || temp.type == TokenType::Number ||
                                 temp.type == TokenType::String)
                            block.push_back(atom(temp));
                        
                        // TODO parse 'then'

                        // TODO parse 'else'
                    }
                    else if (token.token == "let")
                    {}
                    else if (token.token == "mut")
                    {}
                    else if (token.token == "set")
                    {}
                    else if (token.token == "fun")
                    {}
                    else if (token.token == "while")
                    {}
                    else if (token.token == "begin")
                    {}
                    else if (token.token == "import")
                    {}
                    else if (token.token == "quote")
                    {}
                }
                else if (token.type == TokenType::Identifier || token.type == TokenType::Operator)
                {}
                else if (token.type == TokenType::Shorthand)
                {
                    if (token.token == "'")
                    {}
                }
                else
                    throw std::runtime_error("ParseError: can not create block from token " + tokens.front().token);
            }
            
            // pop the ")"
            tokens.pop_front();

            return block;
        }
        
        throw std::runtime_error("ParseError: single expressions are not allowed");
    }

    Token Parser::nextToken(std::list<Token>& tokens)
    {
        const Token out = tokens.front();
        tokens.pop_front();
        return out;
    }

    Node Parser::atom(const Token& token)
    {
        if (token.type == TokenType::Number)
        {
            auto n = Node(std::stod(token.token));
            n.setPos(token.line, token.col);
            return n;
        }
        else if (token.type == TokenType::String)
        {
            std::string str = token.token;
            // remove the " at the beginning and at the end
            str.erase(0, 1);
            str.erase(token.token.size() - 2, 1);

            auto n = Node(str);
            n.setPos(token.line, token.col);
            return n;
        }
        else if (token.type == TokenType::Keyword)
        {
            auto n = Node(Keyword::);
            n.setPos(token.line, token.col);
            return n;
        }

        // assuming it is a symbol
        auto n = Node(NodeType::Symbol);
        n.setString(token.token);
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
                            file = n.const_list()[1].string();
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
                        n.list().emplace_back(Keyword::Begin);

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
                                    throw std::runtime_error("ParseError: Couldn't find file " + file);
                            }

                            n.list().push_back(p.ast());
                        }
                        else
                            throw std::runtime_error("ParseError: Cyclic inclusion issue: file " + m_file + " is trying to include " + path + " which was already included");
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

                if (it->nodeType() == NodeType::Symbol && it->string() == "'")
                {
                    Node temp(NodeType::List);

                    temp.push_back(Node(Keyword::Quote));
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
