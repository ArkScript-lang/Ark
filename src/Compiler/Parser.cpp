#include <Ark/Compiler/Parser.hpp>

#include <optional>
#include <algorithm>

#include <Ark/Log.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Builtins/Builtins.hpp>

namespace Ark
{
    using namespace Ark::internal;

    Parser::Parser(unsigned debug, const std::string& lib_dir, uint16_t options) noexcept :
        m_debug(debug),
        m_libdir(lib_dir),
        m_options(options),
        m_lexer(debug),
        m_file(ARK_NO_NAME_FILE)
    {}

    void Parser::feed(const std::string& code, const std::string& filename)
    {
        // not the default value
        if (filename != ARK_NO_NAME_FILE)
        {
            m_file = Ark::Utils::canonicalRelPath(filename);
            if (m_debug >= 2)
                Ark::logger.data("New parser:", m_file);
            m_parent_include.push_back(m_file);
        }

        m_code = code;

        m_lexer.feed(code);
        // apply syntactic sugar
        std::vector<Token> t = m_lexer.tokens();
        if (t.empty())
            throwParseError_("empty file");
        sugar(t);

        // create program
        std::list<Token> tokens(t.begin(), t.end());
        m_last_token = tokens.front();

        // accept every nodes in the file
        m_ast = Node(NodeType::List);
        m_ast.setFilename(m_file);
        m_ast.list().emplace_back(Keyword::Begin);
        while (!tokens.empty())
            m_ast.list().push_back(parse(tokens));
        // include files if needed
        checkForInclude(m_ast);

        if (m_debug >= 3)
        {
            Ark::logger.info("(Parser) AST:");
            std::cout << m_ast << std::endl << std::endl;
        }
    }

    const Node& Parser::ast() const noexcept
    {
        return m_ast;
    }

    const std::vector<std::string>& Parser::getImports() const noexcept
    {
        return m_parent_include;
    }

    void Parser::sugar(std::vector<Token>& tokens) noexcept
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
                tokens.insert(tokens.begin() + i + 1, Token(TokenType::Identifier, "list", line, col));
            }

            ++i;

            if (i == tokens.size())
                break;
        }
    }

    // sugar() was called before, so it's safe to assume we only have ( and )
    Node Parser::parse(std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read)
    {
        using namespace std::string_literals;

        Token token = nextToken(tokens);

        // parse block
        if (token.token == "(")
        {
            bool previous_token_was_lparen = true;

            // create a list node to host the block
            Node block(NodeType::List);
            block.setPos(token.line, token.col);
            block.setFilename(m_file);

            // handle sub-blocks
            if (tokens.front().token == "(")
            {
                block.push_back(parse(tokens));
                previous_token_was_lparen = false;
            }

            // take next token, we don't want to play with a "("
            token = nextToken(tokens);

            // return an empty block
            if (token.token == ")")
                return block;

            // loop until we reach the end of the block
            do
            {
                Node atomized = atom(token);

                // error reporting
                if ((atomized.nodeType() == NodeType::String || atomized.nodeType() == NodeType::Number ||
                        atomized.nodeType() == NodeType::List) && previous_token_was_lparen)
                {
                    std::stringstream ss;
                    ss << "found invalid token after `(', expected Keyword, Identifier";
                    if (!authorize_capture && !authorize_field_read)
                        ss << " or Operator";
                    else
                    {
                        ss << ", Operator";
                        if (authorize_capture && !authorize_field_read)
                            ss << " or Capture";
                        else if (!authorize_capture && authorize_field_read)
                            ss << " or GetField";
                        else
                            ss << ", Capture or GetField";
                    }
                    throwParseError(ss.str(), token);
                }

                block.push_back(atomized);

                expect(!tokens.empty(), "expected more tokens after `" + token.token + "'", m_last_token);
                m_last_token = tokens.front();

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
                            block.push_back(atom(nextToken(tokens)));
                        else
                            throwParseError("found invalid token after keyword `if', expected function call, value or Identifier", temp);
                        // parse 'then'
                        expect(!tokens.empty() && tokens.front().token != ")", "expected a statement after the condition", temp);
                        block.push_back(parse(tokens));
                        // parse 'else', if there is one
                        if (tokens.front().token != ")")
                        {
                            block.push_back(parse(tokens));
                            // error handling if the if is ill-formed
                            expect(tokens.front().token == ")", "if block is ill-formed, got more than the 3 required arguments (condition, then, else)", m_last_token);
                        }
                    }
                    else if (token.token == "let" || token.token == "mut")
                    {
                        auto temp = tokens.front();
                        // parse identifier
                        if (temp.type == TokenType::Identifier)
                            block.push_back(atom(nextToken(tokens)));
                        else
                            throwParseError(std::string("missing identifier to define a ") + (token.token == "let" ? "constant" : "variable") + ", after keyword `" + token.token + "'", temp);
                        expect(!tokens.empty() && tokens.front().token != ")", "expected a value after the identifier", temp);
                        // value
                        while (tokens.front().token != ")")
                            block.push_back(parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ true));

                        // the block size can exceed 3 only if we have a serie of getfields
                        expect(
                            block.list().size() <= 3 || std::all_of(block.list().begin() + 3, block.list().end(), [](const Node& n) -> bool { return n.nodeType() == NodeType::GetField; }),
                            "too many arguments given to keyword `" + token.token + "', got " + Utils::toString(block.list().size() - 1) + ", expected at most 3",
                            m_last_token
                        );
                    }
                    else if (token.token == "set")
                    {
                        auto temp = tokens.front();
                        // parse identifier
                        if (temp.type == TokenType::Identifier)
                            block.push_back(atom(nextToken(tokens)));
                        else
                            throwParseError("missing identifier to assign a value to, after keyword `set'", temp);
                        expect(!tokens.empty() && tokens.front().token != ")", "expected a value after the identifier", temp);
                        // set can not accept a.b...c as an identifier
                        if (tokens.front().type == TokenType::GetField)
                            throwParseError("found invalid token after keyword `set', expected an identifier, got a closure field reading expression", tokens.front());
                        // value
                        while (tokens.front().token != ")")
                            block.push_back(parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ true));

                        // the block size can exceed 3 only if we have a serie of getfields
                        expect(
                            block.list().size() <= 3 || std::all_of(block.list().begin() + 3, block.list().end(), [](const Node& n) -> bool { return n.nodeType() == NodeType::GetField; }),
                            "too many arguments given to keyword `" + token.token + "', got " + Utils::toString(block.list().size() - 1) + ", expected at most 3",
                            m_last_token
                        );
                    }
                    else if (token.token == "fun")
                    {
                        // parse arguments
                        if (tokens.front().type == TokenType::Grouping)
                            block.push_back(parse(tokens, /* authorize_capture */ true));
                        else
                            throwParseError("found invalid token after keyword `fun', expected a block to define the argument list of the function\nThe block can be empty if it doesn't have arguments: `()'", tokens.front());
                        // parse body
                        if (tokens.front().type == TokenType::Grouping)
                            block.push_back(parse(tokens));
                        else
                            throwParseError("the body of a function must be a block, even an empty one `()'", tokens.front());
                        expect(block.list().size() == 3, "got too many arguments after keyword `" + token.token + "', expected an argument list and a body", m_last_token);
                    }
                    else if (token.token == "while")
                    {
                        auto temp = tokens.front();
                        // parse condition
                        if (temp.type == TokenType::Grouping)
                            block.push_back(parse(tokens));
                        else if (temp.type == TokenType::Identifier || temp.type == TokenType::Number ||
                                 temp.type == TokenType::String)
                            block.push_back(atom(nextToken(tokens)));
                        else
                            throwParseError("found invalid token after keyword `while', expected function call, value or Identifier", temp);
                        expect(!tokens.empty() && tokens.front().token != ")", "expected a body after the condition", temp);
                        // parse 'do'
                        block.push_back(parse(tokens));
                        expect(block.list().size() == 3, "got too many arguments after keyword `" + token.token + "', expected a condition and a body", temp);
                    }
                    else if (token.token == "begin")
                    {
                        while (true)
                        {
                            expect(!tokens.empty(), "a `begin' block was opened but never closed\nYou most likely forgot a `}' or `)'", m_last_token);
                            if (tokens.front().token == ")")
                                break;
                            m_last_token = tokens.front();

                            block.push_back(parse(tokens));
                        }
                    }
                    else if (token.token == "import")
                    {
                        if (tokens.front().type == TokenType::String)
                            block.push_back(atom(nextToken(tokens)));
                        else
                            throwParseError("found invalid token after keyword `import', expected String (path to the file or module to import)", tokens.front());
                        expect(tokens.front().token == ")", "got too many arguments after keyword `import', expected a single filename as String", tokens.front());
                    }
                    else if (token.token == "quote")
                    {
                        block.push_back(parse(tokens));
                        expect(tokens.front().token == ")", "got too many arguments after keyword `quote', expected a single block or value", tokens.front());
                    }
                    else if (token.token == "del")
                    {
                        if (tokens.front().type == TokenType::Identifier)
                            block.push_back(atom(nextToken(tokens)));
                        else
                            throwParseError("found invalid token after keyword `del', expected Identifier", tokens.front());
                        expect(tokens.front().token == ")", "got too many arguments after keyword `del', expected a single identifier", tokens.front());
                    }
                }
                else if (token.type == TokenType::Identifier || token.type == TokenType::Operator ||
                        (token.type == TokenType::Capture && authorize_capture) ||
                        (token.type == TokenType::GetField && authorize_field_read))
                {
                    while (tokens.front().token != ")")
                        block.push_back(parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ true));
                }
            } while (tokens.front().token != ")");

            // pop the ")"
            tokens.pop_front();
            return block;
        }
        else if (token.type == TokenType::Shorthand)
        {
            if (token.token == "'")
            {
                // create a list node to host the block
                Node block(NodeType::List);
                block.setPos(token.line, token.col);
                block.setFilename(m_file);

                block.push_back(Node(Keyword::Quote));
                block.list().back().setPos(token.line, token.col);
                block.list().back().setFilename(m_file);
                block.push_back(parse(tokens));
                return block;
            }
            else
                throwParseError("unknown shorthand", token);
        }
        // error, we shouldn't have grouping token here
        else if (token.type == TokenType::Grouping)
        {
            throwParseError("Found a lonely `" + token.token + "', you most likely have too much parenthesis.", token);
        }
        else if ((token.type == TokenType::Operator || token.type == TokenType::Identifier) &&
                 std::find(Builtins::operators.begin(), Builtins::operators.end(), token.token) != Builtins::operators.end())
        {
            throwParseError("Found a free flying operator, which isn't authorized. Operators should always immediatly follow a `('.", token);
        }
        return atom(token);
    }

    Token Parser::nextToken(std::list<Token>& tokens)
    {
        expect(!tokens.empty(), "no more token to consume", m_last_token);
        m_last_token = tokens.front();

        const Token out = std::move(tokens.front());
        tokens.pop_front();
        return out;
    }

    Node Parser::atom(const Token& token)
    {
        if (token.type == TokenType::Number)
        {
            auto n = Node(std::stod(token.token));
            n.setPos(token.line, token.col);
            n.setFilename(m_file);
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
            n.setFilename(m_file);
            return n;
        }
        else if (token.type == TokenType::Keyword)
        {
            std::optional<Keyword> kw;
            if (token.token == "if")          kw = Keyword::If;
            else if (token.token == "set")    kw = Keyword::Set;
            else if (token.token == "let")    kw = Keyword::Let;
            else if (token.token == "mut")    kw = Keyword::Mut;
            else if (token.token == "fun")    kw = Keyword::Fun;
            else if (token.token == "while")  kw = Keyword::While;
            else if (token.token == "begin")  kw = Keyword::Begin;
            else if (token.token == "import") kw = Keyword::Import;
            else if (token.token == "quote")  kw = Keyword::Quote;
            else if (token.token == "del")    kw = Keyword::Del;
            if (kw)
            {
                auto n = Node(kw.value());
                n.setPos(token.line, token.col);
                n.setFilename(m_file);
                return n;
            }
            throwParseError("unknown keyword", token);
        }
        else if (token.type == TokenType::Capture)
        {
            auto n = Node(NodeType::Capture);
            n.setString(token.token);
            n.setPos(token.line, token.col);
            n.setFilename(m_file);
            return n;
        }
        else if (token.type == TokenType::GetField)
        {
            auto n = Node(NodeType::GetField);
            n.setString(token.token);
            n.setPos(token.line, token.col);
            n.setFilename(m_file);
            return n;
        }

        // assuming it is a TokenType::Identifier, thus a Symbol
        auto n = Node(NodeType::Symbol);
        n.setString(token.token);
        n.setPos(token.line, token.col);
        n.setFilename(m_file);
        return n;
    }

    // high cpu cost
    bool Parser::checkForInclude(Node& n)
    {
        if (n.nodeType() == NodeType::Keyword)
        {
            if (n.keyword() == Keyword::Import)
                return true;
        }
        else if (n.nodeType() == NodeType::List)
        {
            // can not optimize calls to n.list().size() because we are modifying n.list()
            for (std::size_t i=0; i < n.list().size(); ++i)
            {
                if (checkForInclude(n.list()[i]))
                {
                    if (m_debug >= 2)
                        Ark::logger.info("Import found in file:", m_file);

                    std::string file;
                    if (n.const_list()[1].nodeType() == NodeType::String)
                        file = n.const_list()[1].string();
                    else
                        throw Ark::TypeError("Arguments of import must be of type String");

                    namespace fs = std::filesystem;
                    using namespace std::string_literals;

                    std::string ext = fs::path(file).extension().string();
                    std::string dir = Ark::Utils::getDirectoryFromPath(m_file) + "/";
                    std::string path = (dir != "/") ? dir + file : file;

                    if (m_debug >= 2)
                        Ark::logger.data("path:", path, " ; file:", file, " ; libdir:", m_libdir);

                    // check if we are not loading a plugin
                    if (ext == ".ark")
                    {
                        n.list().clear();
                        // replace content with a begin block
                        n.list().emplace_back(Keyword::Begin);

                        std::string libpath;
                        std::string included_file = "";
                        std::string filename = Ark::Utils::getFilenameFromPath(file);

                        if (m_debug >= 2)
                            Ark::logger.info("filename:", filename);

                        // search in the files of the user first
                        if (Ark::Utils::fileExists(path))
                            included_file = path;
                        else if (libpath = m_libdir + "/std/" + file; Ark::Utils::fileExists(libpath))
                            included_file = libpath;
                        else if (libpath = m_libdir + "/" + file; Ark::Utils::fileExists(libpath))
                            included_file = libpath;
                        else
                            throw std::runtime_error("While processing file " + m_file + ", couldn't import " + file + ": file not found");

                        // if the file isn't in the include list, then we can include it
                        // this avoids cyclic includes
                        if (std::find(m_parent_include.begin(), m_parent_include.end(), Ark::Utils::canonicalRelPath(included_file)) == m_parent_include.end())
                        {
                            Parser p(m_debug, m_libdir, m_options);
                            // feed the new parser with our parent includes
                            for (auto&& pi : m_parent_include)
                                p.m_parent_include.push_back(Ark::Utils::canonicalRelPath(pi));  // new parser, we can assume that the parent include list is empty
                            p.m_parent_include.push_back(Ark::Utils::canonicalRelPath(m_file));  // add the current file to avoid importing it again

                            p.feed(Ark::Utils::readFile(included_file), included_file);

                            // update our list of included files
                            for (auto&& inc : p.m_parent_include)
                            {
                                if (std::find(m_parent_include.begin(), m_parent_include.end(), inc) == m_parent_include.end())
                                    m_parent_include.push_back(Ark::Utils::canonicalRelPath(inc));
                            }

                            n.list().push_back(p.ast());
                        }
                    }
                }
            }
        }

        return false;
    }

    std::ostream& operator<<(std::ostream& os, const Parser& P) noexcept
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
