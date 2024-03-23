#include <Ark/Compiler/AST/Parser.hpp>

#include <Ark/Files.hpp>

namespace Ark::internal
{
    Parser::Parser(bool interpret) :
        BaseParser(), m_interpret(interpret), m_ast(NodeType::List), m_imports({}), m_allow_macro_behavior(0)
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

    const std::vector<Import>& Parser::imports() const
    {
        return m_imports;
    }

    void Parser::run()
    {
        while (!isEOF())
        {
            std::string comment;
            newlineOrComment(&comment);
            if (isEOF())
                break;

            auto n = node();
            if (n)
                m_ast.push_back(n.value().attachNearestCommentBefore(comment));
        }
    }

    void Parser::setNodePosAndFilename(Node& node)
    {
        auto position = getCursor();
        node.setPos(position.row, position.col);
        node.setFilename(m_filename);
    }

    std::optional<Node> Parser::node()
    {
        // save current position in buffer to be able to go back if needed
        auto position = getCount();

        if (auto result = wrapped(&Parser::letMutSet, "let/mut/set"))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::function, "function"))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::condition, "condition"))
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::loop, "loop"))
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

        if (auto result = wrapped(&Parser::macroCondition, "$if"))
            return result;
        else
            backtrack(position);

        if (auto result = macro(); result.has_value())
            return result;
        else
            backtrack(position);

        if (auto result = wrapped(&Parser::del, "del"))
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
        std::string comment;
        newlineOrComment(&comment);

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        if (token == "let")
            leaf.push_back(Node(Keyword::Let));
        else if (token == "mut")
            leaf.push_back(Node(Keyword::Mut));
        else  // "set"
            leaf.push_back(Node(Keyword::Set));

        if (m_allow_macro_behavior > 0)
        {
            auto position = getCount();
            if (auto value = nodeOrValue(); value.has_value())
                leaf.push_back(value.value());
            else
                backtrack(position);
        }

        if (leaf.constList().size() == 1)
        {
            // we haven't parsed anything while in "macro state"
            std::string symbol;
            if (!name(&symbol))
                errorWithNextToken(token + " needs a symbol");

            leaf.push_back(Node(NodeType::Symbol, symbol));
        }

        comment.clear();
        newlineOrComment(&comment);

        if (auto value = nodeOrValue(); value.has_value())
            leaf.push_back(value.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a value");

        return leaf;
    }

    std::optional<Node> Parser::del()
    {
        std::string keyword;
        if (!oneOf({ "del" }, &keyword))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);

        std::string symbol;
        if (!name(&symbol))
            errorWithNextToken(keyword + " needs a symbol");

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(Node(Keyword::Del));
        leaf.push_back(Node(NodeType::Symbol, symbol));

        return leaf;
    }

    std::optional<Node> Parser::condition()
    {
        if (!oneOf({ "if" }))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(Node(Keyword::If));

        if (auto condition = nodeOrValue(); condition.has_value())
            leaf.push_back(condition.value());
        else
            errorWithNextToken("If need a valid condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_true = nodeOrValue(); value_if_true.has_value())
            leaf.push_back(value_if_true.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a value");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_false = nodeOrValue(); value_if_false.has_value())
        {
            leaf.push_back(value_if_false.value().attachNearestCommentBefore(comment));
            newlineOrComment();  // FIXME how to attach a comment after the node? another field?
        }

        return leaf;
    }

    std::optional<Node> Parser::loop()
    {
        if (!oneOf({ "while" }))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(Node(Keyword::While));

        if (auto condition = nodeOrValue(); condition.has_value())
            leaf.push_back(condition.value());
        else
            errorWithNextToken("While need a valid condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto body = nodeOrValue(); body.has_value())
            leaf.push_back(body.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a value");

        return leaf;
    }

    std::optional<Node> Parser::import_()
    {
        if (!accept(IsChar('(')))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf);

        if (!oneOf({ "import" }))
            return std::nullopt;
        comment.clear();
        newlineOrComment(&comment);
        leaf.push_back(Node(Keyword::Import).attachNearestCommentBefore(comment));

        Import import_data;

        if (!packageName(&import_data.prefix))
            errorWithNextToken("Import expected a package name");
        import_data.package.push_back(import_data.prefix);

        Node packageNode(NodeType::List);
        setNodePosAndFilename(packageNode);
        packageNode.push_back(Node(NodeType::String, import_data.prefix));

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
                    import_data.package.push_back(path);
                    import_data.prefix = path;  // in the end we will store the last element of the package, which is what we want
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

        Node symbols(NodeType::List);
        setNodePosAndFilename(symbols);
        // then parse the symbols to import, if any
        comment.clear();
        if (newlineOrComment(&comment))
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

                    symbols.push_back(Node(NodeType::Symbol, symbol).attachNearestCommentBefore(comment));
                    import_data.symbols.push_back(symbol);
                }

                comment.clear();
                if (!newlineOrComment(&comment))  // TODO what to do?
                    break;
            }
        }

        leaf.push_back(packageNode);
        leaf.push_back(symbols);
        // save the import data
        m_imports.push_back(import_data);

        newlineOrComment();  // FIXME: attach comment after the node
        expect(IsChar(')'));
        return leaf;
    }

    std::optional<Node> Parser::block()
    {
        bool alt_syntax = false;
        std::string comment;
        if (accept(IsChar('(')))
        {
            newlineOrComment(&comment);
            if (!oneOf({ "begin" }))
                return std::nullopt;
        }
        else if (accept(IsChar('{')))
            alt_syntax = true;
        else
            return std::nullopt;

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf);
        leaf.push_back(Node(Keyword::Begin).attachNearestCommentBefore(comment));

        comment.clear();
        newlineOrComment(&comment);

        while (!isEOF())
        {
            if (auto value = nodeOrValue(); value.has_value())
            {
                leaf.push_back(value.value().attachNearestCommentBefore(comment));
                comment.clear();
                newlineOrComment(&comment);
            }
            else
                break;
        }

        newlineOrComment(&comment);  // FIXME: attach comment after last node
        expect(IsChar(!alt_syntax ? ')' : '}'));
        return leaf;
    }

    std::optional<Node> Parser::functionArgs()
    {
        expect(IsChar('('));
        std::string comment;
        newlineOrComment(&comment);

        Node args(NodeType::List);
        setNodePosAndFilename(args.attachNearestCommentBefore(comment));
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
                    comment.clear();
                    newlineOrComment(&comment);
                    args.push_back(Node(NodeType::Capture, capture).attachNearestCommentBefore(comment));
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

                    comment.clear();
                    newlineOrComment(&comment);
                    args.push_back(Node(NodeType::Symbol, symbol).attachNearestCommentBefore(comment));
                }
            }
        }

        if (accept(IsChar(')')))
            return args;
        return std::nullopt;
    }

    std::optional<Node> Parser::function()
    {
        if (!oneOf({ "fun" }))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        while (m_allow_macro_behavior > 0)
        {
            auto position = getCount();

            Node leaf(NodeType::List);
            setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
            leaf.push_back(Node(Keyword::Fun));
            // args
            if (auto value = nodeOrValue(); value.has_value())
            {
                // if value is nil, just add an empty argument bloc to prevent bugs when
                // declaring functions inside macros
                Node args = value.value();
                setNodePosAndFilename(args);
                if (args.nodeType() == NodeType::Symbol && args.string() == "nil")
                    leaf.push_back(Node(NodeType::List));
                else
                    leaf.push_back(args);
            }
            else
            {
                backtrack(position);
                break;
            }

            comment.clear();
            newlineOrComment(&comment);
            // body
            if (auto value = nodeOrValue(); value.has_value())
                leaf.push_back(value.value().attachNearestCommentBefore(comment));
            else
                errorWithNextToken("Expected a body for the function");
            return leaf;
        }

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(Node(Keyword::Fun));

        auto position = getCount();
        if (auto args = functionArgs(); args.has_value())
            leaf.push_back(args.value());
        else
        {
            backtrack(position);

            if (auto value = nodeOrValue(); value.has_value())
                leaf.push_back(value.value());
            else
                errorWithNextToken("Expected an argument list");
        }

        comment.clear();
        newlineOrComment(&comment);

        if (auto value = nodeOrValue(); value.has_value())
            leaf.push_back(value.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a body for the function");

        return leaf;
    }

    std::optional<Node> Parser::macroCondition()
    {
        if (!oneOf({ "$if" }))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        Node leaf(NodeType::Macro);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(Node(Keyword::If));

        if (auto condition = nodeOrValue(); condition.has_value())
            leaf.push_back(condition.value());
        else
            errorWithNextToken("$if need a valid condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_true = nodeOrValue(); value_if_true.has_value())
            leaf.push_back(value_if_true.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a value");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_false = nodeOrValue(); value_if_false.has_value())
        {
            leaf.push_back(value_if_false.value().attachNearestCommentBefore(comment));
            comment.clear();
            newlineOrComment(&comment);  // FIXME: attach comment after node
        }

        return leaf;
    }

    std::optional<Node> Parser::macroArgs()
    {
        if (accept(IsChar('(')))
        {
            std::string comment;
            newlineOrComment(&comment);
            Node args = Node(NodeType::List);
            setNodePosAndFilename(args.attachNearestCommentBefore(comment));

            while (!isEOF())
            {
                std::string arg_name;
                if (!name(&arg_name))
                    break;
                else
                {
                    comment.clear();
                    newlineOrComment(&comment);
                    args.push_back(Node(NodeType::Symbol, arg_name).attachNearestCommentBefore(comment));
                }
            }

            if (sequence("..."))
            {
                std::string spread_name;
                if (!name(&spread_name))
                    errorWithNextToken("Expected a name for the variadic arguments list");
                args.push_back(Node(NodeType::Spread, spread_name));
                comment.clear();
                newlineOrComment(&comment);  // FIXME: attach comment after node
            }

            if (!accept(IsChar(')')))
                return std::nullopt;
            comment.clear();
            newlineOrComment(&comment);  // FIXME: attach comment after node

            return args;
        }

        return std::nullopt;
    }

    std::optional<Node> Parser::macro()
    {
        if (!accept(IsChar('(')))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        if (!oneOf({ "$" }))
            return std::nullopt;
        newlineOrComment(&comment);

        Node leaf(NodeType::Macro);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));

        std::string symbol;
        if (!name(&symbol))
            errorWithNextToken("$ needs a symbol to declare a macro");
        comment.clear();
        newlineOrComment(&comment);

        leaf.push_back(Node(NodeType::Symbol, symbol).attachNearestCommentBefore(comment));

        auto position = getCount();
        if (auto args = macroArgs(); args.has_value())
            leaf.push_back(args.value());
        else
        {
            backtrack(position);

            ++m_allow_macro_behavior;
            auto value = nodeOrValue();
            --m_allow_macro_behavior;

            if (value.has_value())
                leaf.push_back(value.value());
            else
                errorWithNextToken("Expected an argument list, atom or node while defining macro `" + symbol + "'");

            if (accept(IsChar(')')))
                return leaf;
        }

        ++m_allow_macro_behavior;
        auto value = nodeOrValue();
        --m_allow_macro_behavior;

        if (value.has_value())
            leaf.push_back(value.value());
        else
            errorWithNextToken("Expected a value while defining macro `" + symbol + "'");

        comment.clear();
        newlineOrComment(&comment);  // FIXME: attach comment after node
        expect(IsChar(')'));
        return leaf;
    }

    std::optional<Node> Parser::functionCall()
    {
        if (!accept(IsChar('(')))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        std::optional<Node> func;
        if (auto atom = anyAtomOf({ NodeType::Symbol, NodeType::Field }); atom.has_value())
            func = atom->attachNearestCommentBefore(comment);
        else if (auto nested = node(); nested.has_value())
            func = nested->attachNearestCommentBefore(comment);
        else
            return std::nullopt;
        comment.clear();
        newlineOrComment(&comment);

        NodeType call_type = NodeType::List;
        if (auto node = func.value(); node.nodeType() == NodeType::Symbol)
        {
            // TODO enhance this to work with more/all macros
            if (node.string() == "$undef")
                call_type = NodeType::Macro;
        }

        Node leaf(call_type);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(func.value());

        while (!isEOF())
        {
            if (auto arg = nodeOrValue(); arg.has_value())
            {
                comment.clear();
                newlineOrComment(&comment);
                leaf.push_back(arg.value().attachNearestCommentBefore(comment));
            }
            else
                break;
        }

        comment.clear();
        newlineOrComment(&comment);  // FIXME: attach comment after node
        expect(IsChar(')'));
        return leaf;
    }

    std::optional<Node> Parser::list()
    {
        if (!accept(IsChar('[')))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        Node leaf(NodeType::List);
        setNodePosAndFilename(leaf.attachNearestCommentBefore(comment));
        leaf.push_back(Node(NodeType::Symbol, "list"));

        comment.clear();
        while (!isEOF())
        {
            if (auto value = nodeOrValue(); value.has_value())
            {
                leaf.push_back(value.value().attachNearestCommentBefore(comment));
                comment.clear();
                newlineOrComment(&comment);
            }
            else
                break;
        }

        newlineOrComment(&comment);
        expect(IsChar(']'));  // FIXME: attach comment after node
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

        if (auto res = Parser::spread(); m_allow_macro_behavior > 0 && res.has_value())
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

    std::optional<Node> Parser::wrapped(std::optional<Node> (Parser::*parser)(), const std::string& name)
    {
        if (!prefix('('))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        if (auto result = (this->*parser)(); result.has_value())
        {
            result->attachNearestCommentBefore(comment);
            comment.clear();
            newlineOrComment(&comment);  // FIXME: attach comment after node
            if (!suffix(')'))
                errorMissingSuffix(')', name);
            return result;
        }

        return std::nullopt;
    }
}
