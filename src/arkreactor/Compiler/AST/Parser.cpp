#include <Ark/Compiler/AST/Parser.hpp>

#include <Ark/Files.hpp>

namespace Ark::internal
{
    Parser::Parser() :
        BaseParser(), m_ast(NodeType::List), m_imports({})
    {
        m_ast.push_back(Node(Keyword::Begin));
    }

    void Parser::processFile(const std::string& filename)
    {
        const std::string code = Utils::readFile(filename);
        initParser(filename, code);
        run();
    }

    void Parser::processString(const std::string& code)
    {
        initParser(ARK_NO_NAME_FILE, code);
        run();
    }

    const Node& Parser::ast() const
    {
        return m_ast;
    }

    void Parser::run()
    {
        while (!isEOF())
        {
            newlineOrComment();
            if (isEOF())
                break;

            auto n = node();
            if (n)
                m_ast.push_back(n.value());
        }
    }

    std::optional<Node> Parser::node()
    {
        // save current position in buffer to be able to go back if needed
        auto position = getCount();

        if (auto result = wrapped(&Parser::letMutSet, "let/mut/set", '(', ')'))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::function, "function", '(', ')'))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::condition, "condition", '(', ')'))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::loop, "loop", '(', ')'))
            return result;
        else
            backtrack(position);

        if (auto result = import_(); result.has_value())
            return result;
        else
            backtrack(position);

        if (auto result = block(); result.has_value())
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::macro, "macro", '(', ')'))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::del, "del", '(', ')'))
            return result;
        else
            backtrack(position);

        if (auto result = functionCall(); result.has_value())
            return result;
        else
            backtrack(position);

        if (auto result = list(); result.has_value())
            return result;
        else
            backtrack(position);

        return std::nullopt;  // will never reach
    }

    std::optional<Node> Parser::letMutSet()
    {
        std::string token;
        if (!oneOf({ "let", "mut", "set" }, &token))
            return std::nullopt;
        newlineOrComment();

        std::string symbol;
        if (!name(&symbol))
            errorWithNextToken(token + " needs a symbol");
        newlineOrComment();

        Node leaf(NodeType::List);
        if (token == "let")
            leaf.push_back(Node(Keyword::Let));
        else if (token == "mut")
            leaf.push_back(Node(Keyword::Mut));
        else  // "set"
            leaf.push_back(Node(Keyword::Set));
        leaf.push_back(Node(NodeType::Symbol, symbol));

        if (auto value = nodeOrValue(); value.has_value())
            leaf.push_back(value.value());
        else
            errorWithNextToken("Expected a value");

        return leaf;
    }

    std::optional<Node> Parser::del()
    {
        std::string keyword;
        if (!oneOf({ "del" }, &keyword))
            return std::nullopt;

        newlineOrComment();

        std::string symbol;
        if (!name(&symbol))
            errorWithNextToken(keyword + " needs a symbol");

        Node leaf(NodeType::List);
        leaf.push_back(Node(Keyword::Del));
        leaf.push_back(Node(NodeType::Symbol, symbol));

        return leaf;
    }

    std::optional<Node> Parser::condition()
    {
        if (!oneOf({ "if" }))
            return std::nullopt;

        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(Node(Keyword::If));

        if (auto condition = nodeOrValue(); condition.has_value())
            leaf.push_back(condition.value());
        else
            errorWithNextToken("If need a valid condition");

        newlineOrComment();

        if (auto value_if_true = nodeOrValue(); value_if_true.has_value())
            leaf.push_back(value_if_true.value());
        else
            errorWithNextToken("Expected a value");

        newlineOrComment();

        if (auto value_if_false = nodeOrValue(); value_if_false.has_value())
        {
            leaf.push_back(value_if_false.value());
            newlineOrComment();
        }

        return leaf;
    }

    std::optional<Node> Parser::loop()
    {
        if (!oneOf({ "while" }))
            return std::nullopt;

        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(Node(Keyword::While));

        if (auto condition = nodeOrValue(); condition.has_value())
            leaf.push_back(condition.value());
        else
            errorWithNextToken("While need a valid condition");

        newlineOrComment();

        if (auto body = nodeOrValue(); body.has_value())
            leaf.push_back(body.value());
        else
            errorWithNextToken("Expected a value");

        return leaf;
    }

    std::optional<Node> Parser::import_()
    {
        if (!accept(IsChar('(')))
            return std::nullopt;
        newlineOrComment();

        if (!oneOf({ "import" }))
            return std::nullopt;
        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(Node(Keyword::Import));

        Import import_data;

        if (!packageName(&import_data.package))
            errorWithNextToken("Import expected a package name");

        Node packageNode(NodeType::List);
        packageNode.push_back(Node(NodeType::String, import_data.package));
        Node symbols(NodeType::List);

        // first, parse the package name
        while (!isEOF())
        {
            // parsing package folder.foo.bar.yes
            if (accept(IsChar('.')))
            {
                std::string path;
                if (!packageName(&path))
                    errorWithNextToken("Package name expected after '.'");
                else
                {
                    packageNode.push_back(Node(NodeType::String, path));
                    import_data.package = path;  // in the end we will store the last element of the package, which is what we want
                }
            }
            else if (accept(IsChar(':')) && accept(IsChar('*')))  // parsing :*
            {
                space();
                expect(IsChar(')'));

                leaf.push_back(packageNode);
                leaf.push_back(Node(NodeType::Symbol, "*"));

                // save the import data structure to know we encounter an import node, and retrieve its data more easily later on
                import_data.with_prefix = false;
                m_imports.push_back(import_data);

                return leaf;
            }
            else
                break;
        }

        // then parse the symbols to import, it any
        if (newlineOrComment())
        {
            while (!isEOF())
            {
                if (accept(IsChar(':')))  // parsing potential :a :b :c
                {
                    std::string symbol;
                    if (!name(&symbol))
                        errorWithNextToken("Expected a valid symbol to import");

                    if (symbol.size() >= 2 && symbol[symbol.size() - 2] == ':' && symbol.back() == '*')
                    {
                        backtrack(getCount() - 2);  // we can backtrack n-2 safely here because we know the previous chars were ":*"
                        error("Glob pattern can not follow a symbol to import", ":*");
                    }

                    symbols.push_back(Node(NodeType::Symbol, symbol));
                    import_data.symbols.push_back(symbol);
                }

                if (!newlineOrComment())
                    break;
            }
        }

        leaf.push_back(packageNode);
        leaf.push_back(symbols);
        // save the import data
        m_imports.push_back(import_data);

        newlineOrComment();
        expect(IsChar(')'));
        return leaf;
    }

    std::optional<Node> Parser::block()
    {
        bool alt_syntax = false;
        if (accept(IsChar('(')))
        {
            newlineOrComment();
            if (!oneOf({ "begin" }))
                return std::nullopt;
        }
        else if (accept(IsChar('{')))
            alt_syntax = true;
        else
            return std::nullopt;
        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(Node(Keyword::Begin));

        while (!isEOF())
        {
            if (auto value = nodeOrValue(); value.has_value())
            {
                leaf.push_back(value.value());
                newlineOrComment();
            }
            else
                break;
        }

        newlineOrComment();
        expect(IsChar(!alt_syntax ? ')' : '}'));
        return leaf;
    }

    std::optional<Node> Parser::function()
    {
        if (!oneOf({ "fun" }))
            return std::nullopt;
        newlineOrComment();

        expect(IsChar('('));
        newlineOrComment();

        Node args(NodeType::List);
        bool has_captures = false;

        while (!isEOF())
        {
            if (accept(IsChar('&')))  // captures
            {
                has_captures = true;
                std::string capture;
                if (!name(&capture))
                    break;
                else
                {
                    newlineOrComment();
                    args.push_back(Node(NodeType::Capture, capture));
                }
            }
            else
            {
                auto pos = getCount();
                std::string symbol;
                if (!name(&symbol))
                    break;
                else
                {
                    if (has_captures)
                    {
                        backtrack(pos);
                        error("Captured variables should be at the end of the argument list", symbol);
                    }

                    newlineOrComment();
                    args.push_back(Node(NodeType::Symbol, symbol));
                }
            }
        }

        expect(IsChar(')'));
        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(Node(Keyword::Fun));
        leaf.push_back(args);

        if (auto value = nodeOrValue(); value.has_value())
            leaf.push_back(value.value());
        else
            errorWithNextToken("Expected a value");

        return leaf;
    }

    std::optional<Node> Parser::macro()
    {
        if (!oneOf({ "macro" }))
            return std::nullopt;
        newlineOrComment();

        // TODO handle if macros
        std::string symbol;
        if (!name(&symbol))
            errorWithNextToken("macro needs a symbol");
        newlineOrComment();

        Node leaf(NodeType::Macro);
        leaf.push_back(Node(NodeType::Symbol, symbol));

        if (accept(IsChar('(')))
        {
            newlineOrComment();
            Node args = Node(NodeType::List);

            while (!isEOF())
            {
                std::string arg_name;
                if (!name(&arg_name))
                    break;
                else
                {
                    newlineOrComment();
                    args.push_back(Node(NodeType::Symbol, arg_name));
                }
            }

            if (sequence("..."))
            {
                std::string spread_name;
                if (!name(&spread_name))
                    errorWithNextToken("Expected a name for the variadic arguments list");
                args.push_back(Node(NodeType::Spread, spread_name));
                newlineOrComment();
            }

            expect(IsChar(')'));
            newlineOrComment();

            leaf.push_back(args);
        }

        if (auto value = nodeOrValue(); value.has_value())
            leaf.push_back(value.value());
        else
            errorWithNextToken("Expected a value");

        return leaf;
    }

    std::optional<Node> Parser::functionCall()
    {
        if (!accept(IsChar('(')))
            return std::nullopt;
        newlineOrComment();

        std::optional<Node> func = std::nullopt;
        if (auto atom = anyAtomOf({ NodeType::Symbol, NodeType::Field }); atom.has_value())
            func = atom;
        else if (auto nested = node(); nested.has_value())
            func = nested;
        else
            return std::nullopt;
        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(func.value());

        while (!isEOF())
        {
            if (auto arg = nodeOrValue(); arg.has_value())
            {
                newlineOrComment();
                leaf.push_back(arg.value());
            }
            else
                break;
        }

        newlineOrComment();
        expect(IsChar(')'));
        return leaf;
    }

    std::optional<Node> Parser::list()
    {
        if (!accept(IsChar('[')))
            return std::nullopt;
        newlineOrComment();

        Node leaf(NodeType::List);
        leaf.push_back(Node(NodeType::Symbol, "list"));

        while (!isEOF())
        {
            if (auto value = nodeOrValue(); value.has_value())
            {
                leaf.push_back(value.value());
                newlineOrComment();
            }
            else
                break;
        }

        newlineOrComment();
        expect(IsChar(']'));
        return leaf;
    }

    std::optional<Node> Parser::atom()
    {
        auto pos = getCount();

        if (auto res = Parser::number(); res.has_value())
            return res;
        else
            backtrack(pos);

        if (auto res = Parser::string(); res.has_value())
            return res;
        else
            backtrack(pos);

        if (auto res = Parser::field(); res.has_value())
            return res;
        else
            backtrack(pos);

        if (auto res = Parser::symbol(); res.has_value())
            return res;
        else
            backtrack(pos);

        if (auto res = Parser::nil(); res.has_value())
            return res;
        else
            backtrack(pos);

        return std::nullopt;
    }

    std::optional<Node> Parser::anyAtomOf(std::initializer_list<NodeType> types)
    {
        auto value = atom();
        if (value.has_value())
        {
            for (auto type : types)
            {
                if (value->nodeType() == type)
                    return value;
            }
        }
        return std::nullopt;
    }

    std::optional<Node> Parser::nodeOrValue()
    {
        if (auto value = atom(); value.has_value())
            return value;
        else if (auto sub_node = node(); sub_node.has_value())
            return sub_node;

        return std::nullopt;
    }

    std::optional<Node> Parser::wrapped(std::optional<Node> (Parser::*parser)(), const std::string& name, char a, char b)
    {
        if (!prefix(a))
            return std::nullopt;

        if (auto result = (this->*parser)(); result.has_value())
        {
            if (!suffix(b))
                errorMissingSuffix(b, name);
            return result;
        }

        return std::nullopt;
    }
}
