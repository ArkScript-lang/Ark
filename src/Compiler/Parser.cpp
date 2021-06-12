#include <Ark/Compiler/Parser.hpp>

#include <optional>
#include <algorithm>

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
                std::cout << "New parser: " << m_file << '\n';
            m_parent_include.push_back(m_file);
        }

        m_code = code;

        m_lexer.feed(code);
        // apply syntactic sugar
        std::vector<Token>& t = m_lexer.tokens();
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
        checkForInclude(m_ast, m_ast);

        if (m_debug >= 3)
            std::cout << "(Parser) AST\n" << m_ast << "\n\n";
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
                // handle macros
                if (i > 0 && tokens[i - 1].token != "!")
                    tokens.insert(tokens.begin() + i + 1, Token(TokenType::Keyword, "begin", line, col));
                else if (i == 0)
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
    Node Parser::parse(std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        using namespace std::string_literals;

        Token token = nextToken(tokens);

        // parse block
        if (token.token == "(")
        {
            bool previous_token_was_lparen = true;
            // create a list node to host the block
            Node block = make_node_list(token.line, token.col, m_file);

            // handle sub-blocks
            if (tokens.front().token == "(")
            {
                block.push_back(parse(tokens, false, false, in_macro));
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
                checkForInvalidTokens(atomized, token, previous_token_was_lparen, authorize_capture, authorize_field_read);
                block.push_back(atomized);

                expect(!tokens.empty(), "expected more tokens after `" + token.token + "'", m_last_token);
                m_last_token = tokens.front();

                if (token.type == TokenType::Keyword)
                {
                    void (Parser::*fun_ptr)(Node&, Token&, std::list<Token>&, bool, bool, bool) = nullptr;
                    if (token.token == "if")
                        fun_ptr = &Parser::parseIf;
                    else if (token.token == "let" || token.token == "mut")
                        fun_ptr = &Parser::parseLetMut;
                    else if (token.token == "set")
                        fun_ptr = &Parser::parseSet;
                    else if (token.token == "fun")
                        fun_ptr = &Parser::parseFun;
                    else if (token.token == "while")
                        fun_ptr = &Parser::parseWhile;
                    else if (token.token == "begin")
                        fun_ptr = &Parser::parseBegin;
                    else if (token.token == "import")
                        fun_ptr = &Parser::parseImport;
                    else if (token.token == "quote")
                        fun_ptr = &Parser::parseQuote;
                    else if (token.token == "del")
                        fun_ptr = &Parser::parseDel;

                    if (fun_ptr != nullptr)
                        (this->*fun_ptr)(block, token, tokens, authorize_capture, authorize_field_read, in_macro);
                    else
                        throwParseError("unimplemented keyword `" + token.token + "'. If you see this error please report it on GitHub.", token);
                }
                else if (token.type == TokenType::Identifier || token.type == TokenType::Operator ||
                        (token.type == TokenType::Capture && authorize_capture) ||
                        (token.type == TokenType::GetField && authorize_field_read) ||
                        (token.type == TokenType::Spread && in_macro))
                {
                    while (tokens.front().token != ")")
                        block.push_back(parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ true, in_macro));
                }
            } while (tokens.front().token != ")");

            // pop the ")"
            tokens.pop_front();
            return block;
        }
        else if (token.type == TokenType::Shorthand)
            return parseShorthand(token, tokens, authorize_capture, authorize_field_read, in_macro);
        // error, we shouldn't have grouping token here
        else if (token.type == TokenType::Grouping)
            throwParseError("Found a lonely `" + token.token + "', you most likely have too much parenthesis.", token);
        else if ((token.type == TokenType::Operator || token.type == TokenType::Identifier) &&
                 std::find(Builtins::operators.begin(), Builtins::operators.end(), token.token) != Builtins::operators.end())
            throwParseError("Found a free flying operator, which isn't authorized. Operators should always immediatly follow a `('.", token);
        return atom(token);
    }

    void Parser::parseIf(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        auto temp = tokens.front();
        // parse condition
        if (temp.type == TokenType::Grouping)
            block.push_back(parse(tokens, false, false, in_macro));
        else if (temp.type == TokenType::Identifier || temp.type == TokenType::Number ||
                    temp.type == TokenType::String || (in_macro && temp.type == TokenType::Spread))
            block.push_back(atom(nextToken(tokens)));
        else
            throwParseError("found invalid token after keyword `if', expected function call, value or Identifier", temp);
        // parse 'then'
        expect(!tokens.empty() && tokens.front().token != ")", "expected a statement after the condition", temp);
        block.push_back(parse(tokens, false, false, in_macro));
        // parse 'else', if there is one
        if (tokens.front().token != ")")
        {
            block.push_back(parse(tokens, false, false, in_macro));
            // error handling if the if is ill-formed
            expect(tokens.front().token == ")", "if block is ill-formed, got more than the 3 required arguments (condition, then, else)", m_last_token);
        }
    }

    void Parser::parseLetMut(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        auto temp = tokens.front();
        // parse identifier
        if (temp.type == TokenType::Identifier)
            block.push_back(atom(nextToken(tokens)));
        else if (in_macro)
            block.push_back(parse(tokens, false, false, in_macro));
        else
            throwParseError(std::string("missing identifier to define a ") + (token.token == "let" ? "constant" : "variable") + ", after keyword `" + token.token + "'", temp);
        expect(!tokens.empty() && tokens.front().token != ")", "expected a value after the identifier", temp);
        // value
        while (tokens.front().token != ")")
            block.push_back(parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ true, in_macro));

        // the block size can exceed 3 only if we have a serie of getfields
        expect(
            block.list().size() <= 3 || std::all_of(block.list().begin() + 3, block.list().end(), [](const Node& n) -> bool { return n.nodeType() == NodeType::GetField; }),
            "too many arguments given to keyword `" + token.token + "', got " + std::to_string(block.list().size() - 1) + ", expected at most 3",
            m_last_token
        );
    }

    void Parser::parseSet(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        auto temp = tokens.front();
        // parse identifier
        if (temp.type == TokenType::Identifier)
            block.push_back(atom(nextToken(tokens)));
        else if (in_macro)
            block.push_back(parse(tokens, false, false, in_macro));
        else
            throwParseError("missing identifier to assign a value to, after keyword `set'", temp);
        expect(!tokens.empty() && tokens.front().token != ")", "expected a value after the identifier", temp);
        // set can not accept a.b...c as an identifier
        if (tokens.front().type == TokenType::GetField)
            throwParseError("found invalid token after keyword `set', expected an identifier, got a closure field reading expression", tokens.front());
        // value
        while (tokens.front().token != ")")
            block.push_back(parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ true, in_macro));

        // the block size can exceed 3 only if we have a serie of getfields
        expect(
            block.list().size() <= 3 || std::all_of(block.list().begin() + 3, block.list().end(), [](const Node& n) -> bool { return n.nodeType() == NodeType::GetField; }),
            "too many arguments given to keyword `" + token.token + "', got " + std::to_string(block.list().size() - 1) + ", expected at most 3",
            m_last_token
        );
    }

    void Parser::parseFun(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        // parse arguments
        if (tokens.front().type == TokenType::Grouping || in_macro)
            block.push_back(parse(tokens, /* authorize_capture */ true, false, in_macro));
        else
            throwParseError("found invalid token after keyword `fun', expected a block to define the argument list of the function\nThe block can be empty if it doesn't have arguments: `()'", tokens.front());
        // parse body
        if (tokens.front().type == TokenType::Grouping || in_macro)
            block.push_back(parse(tokens, false, false, in_macro));
        else
            throwParseError("the body of a function must be a block, even an empty one `()'", tokens.front());
        expect(block.list().size() == 3, "got too many arguments after keyword `" + token.token + "', expected an argument list and a body", m_last_token);
    }

    void Parser::parseWhile(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        auto temp = tokens.front();
        // parse condition
        if (temp.type == TokenType::Grouping)
            block.push_back(parse(tokens, false, false, in_macro));
        else if (temp.type == TokenType::Identifier || temp.type == TokenType::Number ||
                    temp.type == TokenType::String)
            block.push_back(atom(nextToken(tokens)));
        else
            throwParseError("found invalid token after keyword `while', expected function call, value or Identifier", temp);
        expect(!tokens.empty() && tokens.front().token != ")", "expected a body after the condition", temp);
        // parse 'do'
        block.push_back(parse(tokens, false, false, in_macro));
        expect(block.list().size() == 3, "got too many arguments after keyword `" + token.token + "', expected a condition and a body", temp);
    }

    void Parser::parseBegin(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        while (true)
        {
            expect(!tokens.empty(), "a `begin' block was opened but never closed\nYou most likely forgot a `}' or `)'", m_last_token);
            if (tokens.front().token == ")")
                break;
            m_last_token = tokens.front();

            block.push_back(parse(tokens, false, false, in_macro));
        }
    }

    void Parser::parseImport(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        if (tokens.front().type == TokenType::String)
            block.push_back(atom(nextToken(tokens)));
        else
            throwParseError("found invalid token after keyword `import', expected String (path to the file or module to import)", tokens.front());
        expect(tokens.front().token == ")", "got too many arguments after keyword `import', expected a single filename as String", tokens.front());
    }

    void Parser::parseQuote(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        block.push_back(parse(tokens, false, false, in_macro));
        expect(tokens.front().token == ")", "got too many arguments after keyword `quote', expected a single block or value", tokens.front());
    }

    void Parser::parseDel(Node& block, Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        if (tokens.front().type == TokenType::Identifier)
            block.push_back(atom(nextToken(tokens)));
        else
            throwParseError("found invalid token after keyword `del', expected Identifier", tokens.front());
        expect(tokens.front().token == ")", "got too many arguments after keyword `del', expected a single identifier", tokens.front());
    }

    Node Parser::parseShorthand(Token& token, std::list<Token>& tokens, bool authorize_capture, bool authorize_field_read, bool in_macro)
    {
        if (token.token == "'")
        {
            // create a list node to host the block
            Node block = make_node_list(token.line, token.col, m_file);

            block.push_back(make_node(Keyword::Quote, token.line, token.col, m_file));
            block.push_back(parse(tokens, false, false, in_macro));
            return block;
        }
        else if (token.token == "!")
        {
            if (m_debug >= 2)
                std::cout << "Found a macro at " << token.line << ':' << token.col << " in " << m_file << '\n';

            // macros
            Node block = make_node(NodeType::Macro, token.line, token.col, m_file);

            Node parsed = parse(tokens, /* authorize_capture */ false, /* authorize_field_read */ false, /* in_macro */ true);
            if (parsed.nodeType() != NodeType::List || parsed.list().size() < 2 || parsed.list().size() > 4)
                throwParseError("Macros can only defined using the !{ name value } or !{ name (args) value } syntax", token);

            // append the nodes of the parsed node to the current macro node
            for (std::size_t i = 0, end = parsed.list().size(); i < end; ++i)
                block.push_back(parsed.list()[i]);
            return block;
        }

        throwParseError("unknown shorthand", token);

        return Node();
    }

    void Parser::checkForInvalidTokens(Node& atomized, Token& token, bool previous_token_was_lparen, bool authorize_capture, bool authorize_field_read)
    {
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
        switch (token.type)
        {
            case TokenType::Number:
                return make_node(std::stod(token.token), token.line, token.col, m_file);

            case TokenType::String:
            {
                std::string str = token.token;
                // remove the " at the beginning and at the end
                str.erase(0, 1);
                str.erase(token.token.size() - 2, 1);

                return make_node(str, token.line, token.col, m_file);
            }

            case TokenType::Keyword:
            {
                std::optional<Keyword> kw;
                if      (token.token == "if")     kw = Keyword::If;
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
                    return make_node(kw.value(), token.line, token.col, m_file);
                throwParseError("unknown keyword", token);
            }

            case TokenType::Capture:
            case TokenType::GetField:
            case TokenType::Spread:
            {
                Node n = make_node(similar_nodetype_from_tokentype(token.type), token.line, token.col, m_file);
                n.setString(token.type != TokenType::Spread ? token.token : token.token.substr(3));
                return n;
            }

            case TokenType::Shorthand:
                throwParseError("got a shorthand to atomize, and that's not normal. If you see this error please report it on GitHub.", token);

            default:
            {
                // assuming it is a TokenType::Identifier, thus a Symbol
                Node n = make_node(NodeType::Symbol, token.line, token.col, m_file);
                n.setString(token.token);
                return n;
            }
        }
    }

    // high cpu cost
    bool Parser::checkForInclude(Node& n, Node& parent, std::size_t pos)
    {
        namespace fs = std::filesystem;

        if (n.nodeType() == NodeType::List)
        {
            if (n.constList().size() == 0)
                return false;

            const Node& first = n.constList()[0];

            if (first.nodeType() == NodeType::Keyword && first.keyword() == Keyword::Import)
            {
                if (m_debug >= 2)
                    std::cout << "Import found in file: " << m_file << '\n';

                std::string file;
                if (n.constList()[1].nodeType() == NodeType::String)
                    file = n.constList()[1].string();
                else
                    throw Ark::TypeError("Arguments of import must be of type String");

                // check if we are not loading a plugin
                if (fs::path(file).extension().string() == ".ark")
                {
                    // search for the source file everywhere
                    std::string included_file = seekFile(file);

                    // if the file isn't in the include list, then we can include it
                    // this avoids cyclic includes
                    if (std::find(m_parent_include.begin(), m_parent_include.end(), Ark::Utils::canonicalRelPath(included_file)) != m_parent_include.end())
                        return true;

                    Parser p(m_debug, m_libdir, m_options);
                    // feed the new parser with our parent includes
                    for (auto&& pi : m_parent_include)
                        p.m_parent_include.push_back(pi);  // new parser, we can assume that the parent include list is empty
                    p.m_parent_include.push_back(m_file);  // add the current file to avoid importing it again
                    p.feed(Ark::Utils::readFile(included_file), included_file);

                    // update our list of included files
                    for (auto&& inc : p.m_parent_include)
                    {
                        if (std::find(m_parent_include.begin(), m_parent_include.end(), inc) == m_parent_include.end())
                            m_parent_include.push_back(inc);
                    }

                    for (std::size_t j = 1, end = p.ast().constList().size(); j < end; ++j)
                    {
                        parent.list().insert(parent.list().begin() + pos + j, p.ast().constList()[j]);
                    }

                    return true;
                }
            }

            for (std::size_t i = 0; i < n.list().size(); ++i)
            {
                if (checkForInclude(n.list()[i], n, i))
                {
                    n.list().erase(n.list().begin() + i);
                    --i;
                }
            }
        }

        return false;
    }

    std::string Parser::seekFile(const std::string& file)
    {
        const std::string current_dir = Ark::Utils::getDirectoryFromPath(m_file) + "/";
        const std::string path = (current_dir != "/") ? current_dir + file : file;

        if (m_debug >= 2)
        {
            std::cout << "path: " << path << " ; file: " << file << " ; libdir: " << m_libdir << '\n';
            std::cout << "filename: " << Ark::Utils::getFilenameFromPath(file) << '\n';
        }

        // search in the current directory
        if (Ark::Utils::fileExists(path))
            return path;
        // then search in the standard library directory
        else if (std::string f = m_libdir + "/std/" + file; Ark::Utils::fileExists(f))
            return f;
        // then in the standard library root directory
        else if (std::string f = m_libdir + "/" + file;     Ark::Utils::fileExists(f))
            return f;

        // fallback, we couldn't find the file
        throw std::runtime_error("While processing file " + m_file + ", couldn't import " + file + ": file not found");
    }

    std::ostream& operator<<(std::ostream& os, const Parser& P) noexcept
    {
        os << "AST\n";
        if (P.ast().nodeType() == NodeType::List)
        {
            int i = 0;
            for (const auto& node : P.ast().constList())
                std::cout << (i++) << ": " << node << '\n';
        }
        else
            os << "Single item\n" << P.m_ast << std::endl;
        return os;
    }
}
