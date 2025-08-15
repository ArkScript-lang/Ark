#include <Ark/Compiler/AST/Parser.hpp>

#include <fmt/core.h>

namespace Ark::internal
{
    Parser::Parser(const unsigned debug, const bool interpret) :
        BaseParser(), m_interpret(interpret), m_logger("Parser", debug),
        m_ast(NodeType::List), m_imports({}), m_allow_macro_behavior(0),
        m_nested_nodes(0)
    {
        m_ast.push_back(Node(Keyword::Begin));

        m_parsers = {
            [this](FilePosition) {
                return wrapped(&Parser::letMutSet, "variable assignment or declaration");
            },
            [this](FilePosition) {
                return wrapped(&Parser::function, "function");
            },
            [this](FilePosition) {
                return wrapped(&Parser::condition, "condition");
            },
            [this](FilePosition) {
                return wrapped(&Parser::loop, "loop");
            },
            [this](const FilePosition filepos) {
                return import_(filepos);
            },
            [this](const FilePosition filepos) {
                return block(filepos);
            },
            [this](FilePosition) {
                return wrapped(&Parser::macroCondition, "$if");
            },
            [this](const FilePosition filepos) {
                return macro(filepos);
            },
            [this](FilePosition) {
                return wrapped(&Parser::del, "del");
            },
            [this](const FilePosition filepos) {
                return functionCall(filepos);
            },
            [this](const FilePosition filepos) {
                return list(filepos);
            }
        };
    }

    void Parser::process(const std::string& filename, const std::string& code)
    {
        m_logger.traceStart("process");
        initParser(filename, code);

        while (!isEOF())
        {
            std::string comment;
            newlineOrComment(&comment);
            if (isEOF())
            {
                if (!comment.empty())
                    m_ast.list().back().attachCommentAfter(comment);
                break;
            }

            const auto pos = getCount();
            if (auto n = node())
            {
                m_ast.push_back(n->attachNearestCommentBefore(n->comment() + comment));
                comment.clear();
                if (spaceComment(&comment))
                    m_ast.list().back().attachCommentAfter(comment);
            }
            else
            {
                backtrack(pos);
                std::string out = peek();
                std::string message;
                if (out == ")")
                    message = "Unexpected closing paren";
                else if (out == "}")
                    message = "Unexpected closing bracket";
                else if (out == "]")
                    message = "Unexpected closing square bracket";
                else
                    errorWithNextToken("invalid syntax, expected node");
                errorWithNextToken(message);
            }
        }

        m_logger.traceEnd();
    }

    const Node& Parser::ast() const noexcept
    {
        return m_ast;
    }

    const std::vector<Import>& Parser::imports() const
    {
        return m_imports;
    }

    Node Parser::positioned(Node node, const FilePosition cursor) const
    {
        const auto [row, col] = cursor;
        node.setPos(row, col);
        node.setFilename(m_filename);
        return node;
    }

    std::optional<Node>& Parser::positioned(std::optional<Node>& node, const FilePosition cursor) const
    {
        if (!node)
            return node;

        const auto [row, col] = cursor;
        node->setPos(row, col);
        node->setFilename(m_filename);
        return node;
    }

    std::optional<Node> Parser::node()
    {
        ++m_nested_nodes;

        if (m_nested_nodes > MaxNestedNodes)
            errorWithNextToken(fmt::format("Too many nested node while parsing, exceeds limit of {}. Consider rewriting your code by breaking it in functions and macros.", MaxNestedNodes));

        // save current position in buffer to be able to go back if needed
        const auto position = getCount();
        const auto filepos = getCursor();
        std::optional<Node> result = std::nullopt;

        for (auto&& parser : m_parsers)
        {
            result = parser(filepos);

            if (result)
                break;
            backtrack(position);
        }

        // return std::nullopt only on parsing error, nothing matched, the user provided terrible code
        --m_nested_nodes;
        return result;
    }

    std::optional<Node> Parser::letMutSet(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        std::string token;
        if (!oneOf({ "let", "mut", "set" }, &token))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);
        leaf->attachNearestCommentBefore(comment);

        if (token == "let")
            leaf->push_back(Node(Keyword::Let));
        else if (token == "mut")
            leaf->push_back(Node(Keyword::Mut));
        else  // "set"
            leaf->push_back(Node(Keyword::Set));

        if (m_allow_macro_behavior > 0)
        {
            const auto position = getCount();
            if (const auto value = nodeOrValue(); value.has_value())
            {
                const auto sym = value.value();
                if (sym.nodeType() == NodeType::List || sym.nodeType() == NodeType::Symbol || sym.nodeType() == NodeType::Macro || sym.nodeType() == NodeType::Spread)
                    leaf->push_back(sym);
                else
                    error(fmt::format("Can not use a {} as a symbol name, even in a macro", nodeTypes[static_cast<std::size_t>(sym.nodeType())]), sym.repr());
            }
            else
                backtrack(position);
        }

        if (leaf->constList().size() == 1)
        {
            // we haven't parsed anything while in "macro state"
            std::string symbol_name;
            if (!name(&symbol_name))
                errorWithNextToken(token + " needs a symbol");

            leaf->push_back(Node(NodeType::Symbol, symbol_name));
        }

        comment.clear();
        newlineOrComment(&comment);

        if (auto value = nodeOrValue(); value.has_value())
            leaf->push_back(value.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a value");

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::del(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        if (!oneOf({ "del" }))
            return std::nullopt;
        leaf->push_back(Node(Keyword::Del));

        std::string comment;
        newlineOrComment(&comment);

        std::string symbol_name;
        if (!name(&symbol_name))
            errorWithNextToken("del needs a symbol");

        leaf->push_back(Node(NodeType::Symbol, symbol_name));
        leaf->list().back().attachNearestCommentBefore(comment);

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::condition(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        if (!oneOf({ "if" }))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);

        leaf->push_back(Node(Keyword::If));

        if (auto cond_expr = nodeOrValue(); cond_expr.has_value())
            leaf->push_back(cond_expr.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("`if' needs a valid condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_true = nodeOrValue(); value_if_true.has_value())
            leaf->push_back(value_if_true.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a node or value after condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_false = nodeOrValue(); value_if_false.has_value())
        {
            leaf->push_back(value_if_false.value().attachNearestCommentBefore(comment));
            comment.clear();
            if (newlineOrComment(&comment))
                leaf->list().back().attachCommentAfter(comment);
        }
        else if (!comment.empty())
            leaf->attachCommentAfter(comment);

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::loop(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        if (!oneOf({ "while" }))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);

        leaf->push_back(Node(Keyword::While));

        if (auto cond_expr = nodeOrValue(); cond_expr.has_value())
            leaf->push_back(cond_expr.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("`while' needs a valid condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto body = nodeOrValue(); body.has_value())
            leaf->push_back(body.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a node or value after loop condition");

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::import_(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        auto context = generateErrorContext("(");
        if (!accept(IsChar('(')))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);
        leaf->attachNearestCommentBefore(comment);

        if (!oneOf({ "import" }))
            return std::nullopt;
        comment.clear();
        newlineOrComment(&comment);
        leaf->push_back(Node(Keyword::Import));

        Import import_data;
        import_data.col = filepos.col;
        import_data.line = filepos.row;

        const auto pos = getCount();
        if (!packageName(&import_data.prefix))
            errorWithNextToken("Import expected a package name");

        if (import_data.prefix.size() > 255)
        {
            backtrack(pos);
            errorWithNextToken(fmt::format("Import name too long, expected at most 255 characters, got {}", import_data.prefix.size()));
        }
        import_data.package.push_back(import_data.prefix);

        Node packageNode = positioned(Node(NodeType::List), getCursor()).attachNearestCommentBefore(comment);
        packageNode.push_back(Node(NodeType::Symbol, import_data.prefix));

        // first, parse the package name
        while (!isEOF())
        {
            const auto item_pos = getCursor();

            // parsing package folder.foo.bar.yes
            if (accept(IsChar('.')))
            {
                const auto package_pos = getCursor();
                std::string path;
                if (!packageName(&path))
                    errorWithNextToken("Package name expected after '.'");
                else
                {
                    packageNode.push_back(positioned(Node(NodeType::Symbol, path), package_pos));

                    import_data.package.push_back(path);
                    import_data.prefix = path;  // in the end we will store the last element of the package, which is what we want

                    if (path.size() > 255)
                    {
                        backtrack(pos);
                        errorWithNextToken(fmt::format("Import name too long, expected at most 255 characters, got {}", path.size()));
                    }
                }
            }
            else if (accept(IsChar(':')) && accept(IsChar('*')))  // parsing :*, terminal in imports
            {
                leaf->push_back(packageNode);
                leaf->push_back(positioned(Node(NodeType::Symbol, "*"), item_pos));

                space();
                expectSuffixOrError(')', fmt::format("in import `{}'", import_data.toPackageString()), context);

                // save the import data structure to know we encounter an import node, and retrieve its data more easily later on
                import_data.with_prefix = false;
                import_data.is_glob = true;
                m_imports.push_back(import_data);

                return positioned(leaf, filepos);
            }
            else
                break;
        }

        Node symbols = positioned(Node(NodeType::List), getCursor());
        // then parse the symbols to import, if any
        if (space())
        {
            comment.clear();
            newlineOrComment(&comment);

            while (!isEOF())
            {
                if (accept(IsChar(':')))  // parsing potential :a :b :c
                {
                    const auto symbol_pos = getCursor();
                    std::string symbol_name;
                    if (!name(&symbol_name))
                        errorWithNextToken("Expected a valid symbol to import");
                    if (symbol_name == "*")
                        error(fmt::format("Glob patterns can not be separated from the package, use (import {}:*) instead", import_data.toPackageString()), symbol_name);

                    if (symbol_name.size() >= 2 && symbol_name[symbol_name.size() - 2] == ':' && symbol_name.back() == '*')
                    {
                        backtrack(getCount() - 2);  // we can backtrack n-2 safely here because we know the previous chars were ":*"
                        error("Glob pattern can not follow a symbol to import", ":*");
                    }

                    symbols.push_back(positioned(Node(NodeType::Symbol, symbol_name).attachNearestCommentBefore(comment), symbol_pos));
                    comment.clear();

                    import_data.symbols.push_back(symbol_name);
                    // we do not need the prefix when importing specific symbols
                    import_data.with_prefix = false;
                }

                if (!space())
                    break;
                comment.clear();
                newlineOrComment(&comment);
            }

            if (!comment.empty() && !symbols.list().empty())
                symbols.list().back().attachCommentAfter(comment);
        }

        leaf->push_back(packageNode);
        leaf->push_back(symbols);
        // save the import data
        m_imports.push_back(import_data);

        comment.clear();
        if (newlineOrComment(&comment))
            leaf->list().back().attachCommentAfter(comment);

        expectSuffixOrError(')', fmt::format("in import `{}'", import_data.toPackageString()), context);
        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::block(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        auto context = generateErrorContext("(");
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

        leaf->setAltSyntax(alt_syntax);
        leaf->push_back(Node(Keyword::Begin).attachNearestCommentBefore(comment));

        comment.clear();
        newlineOrComment(&comment);

        while (!isEOF())
        {
            if (auto value = nodeOrValue(); value.has_value())
            {
                leaf->push_back(value.value().attachNearestCommentBefore(comment));
                comment.clear();
                newlineOrComment(&comment);
            }
            else
                break;
        }

        newlineOrComment(&comment);
        expectSuffixOrError(alt_syntax ? '}' : ')', "to close block", context);
        leaf->list().back().attachCommentAfter(comment);
        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::functionArgs(const FilePosition filepos)
    {
        expect(IsChar('('));
        std::optional<Node> args { NodeType::List };

        std::string comment;
        newlineOrComment(&comment);
        args->attachNearestCommentBefore(comment);

        bool has_captures = false;

        while (!isEOF())
        {
            const auto pos = getCursor();
            if (accept(IsChar('&')))  // captures
            {
                has_captures = true;
                std::string capture;
                if (!name(&capture))
                    break;

                args->push_back(positioned(Node(NodeType::Capture, capture), pos));
            }
            else
            {
                const auto count = getCount();
                std::string symbol_name;
                if (!name(&symbol_name))
                    break;
                if (has_captures)
                {
                    backtrack(count);
                    error("Captured variables should be at the end of the argument list", symbol_name);
                }

                args->push_back(positioned(Node(NodeType::Symbol, symbol_name), pos));
            }

            if (!comment.empty())
                args->list().back().attachNearestCommentBefore(comment);
            comment.clear();
            newlineOrComment(&comment);
        }

        if (accept(IsChar(')')))
            return positioned(args, filepos);
        return std::nullopt;
    }

    std::optional<Node> Parser::function(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        if (!oneOf({ "fun" }))
            return std::nullopt;
        leaf->push_back(Node(Keyword::Fun));

        std::string comment_before_args;
        newlineOrComment(&comment_before_args);

        while (m_allow_macro_behavior > 0)
        {
            const auto position = getCount();

            // args
            if (const auto value = nodeOrValue(); value.has_value())
            {
                // if value is nil, just add an empty argument bloc to prevent bugs when
                // declaring functions inside macros
                Node args = value.value();
                if (args.nodeType() == NodeType::Symbol && args.string() == "nil")
                    leaf->push_back(Node(NodeType::List));
                else
                    leaf->push_back(args);
            }
            else
            {
                backtrack(position);
                break;
            }

            std::string comment;
            newlineOrComment(&comment);
            // body
            if (auto value = nodeOrValue(); value.has_value())
                leaf->push_back(value.value().attachNearestCommentBefore(comment));
            else
                errorWithNextToken("Expected a body for the function");
            return positioned(leaf, filepos);
        }

        const auto position = getCount();
        const auto args_file_pos = getCursor();
        if (auto args = functionArgs(args_file_pos); args.has_value())
            leaf->push_back(args.value().attachNearestCommentBefore(comment_before_args));
        else
        {
            backtrack(position);

            if (auto value = nodeOrValue(); value.has_value())
                leaf->push_back(value.value().attachNearestCommentBefore(comment_before_args));
            else
                errorWithNextToken("Expected an argument list");
        }

        std::string comment;
        newlineOrComment(&comment);

        if (auto value = nodeOrValue(); value.has_value())
            leaf->push_back(value.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a body for the function");

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::macroCondition(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::Macro };

        if (!oneOf({ "$if" }))
            return std::nullopt;
        leaf->push_back(Node(Keyword::If));

        std::string comment;
        newlineOrComment(&comment);
        leaf->attachNearestCommentBefore(comment);

        if (const auto cond_expr = nodeOrValue(); cond_expr.has_value())
            leaf->push_back(cond_expr.value());
        else
            errorWithNextToken("$if need a valid condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_true = nodeOrValue(); value_if_true.has_value())
            leaf->push_back(value_if_true.value().attachNearestCommentBefore(comment));
        else
            errorWithNextToken("Expected a node or value after condition");

        comment.clear();
        newlineOrComment(&comment);

        if (auto value_if_false = nodeOrValue(); value_if_false.has_value())
        {
            leaf->push_back(value_if_false.value().attachNearestCommentBefore(comment));
            comment.clear();
            newlineOrComment(&comment);
            leaf->list().back().attachCommentAfter(comment);
        }

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::macroArgs(const FilePosition filepos)
    {
        if (!accept(IsChar('(')))
            return std::nullopt;

        std::optional<Node> args { NodeType::List };

        std::string comment;
        newlineOrComment(&comment);
        args->attachNearestCommentBefore(comment);

        std::vector<std::string> names;
        while (!isEOF())
        {
            const auto pos = getCount();

            std::string arg_name;
            if (!name(&arg_name))
                break;
            comment.clear();
            newlineOrComment(&comment);
            args->push_back(Node(NodeType::Symbol, arg_name).attachNearestCommentBefore(comment));

            if (std::ranges::find(names, arg_name) != names.end())
            {
                backtrack(pos);
                errorWithNextToken(fmt::format("Argument names must be unique, can not reuse `{}'", arg_name));
            }
            names.push_back(arg_name);
        }

        const auto pos = getCount();
        if (sequence("..."))
        {
            std::string spread_name;
            if (!name(&spread_name))
                errorWithNextToken("Expected a name for the variadic arguments list");
            args->push_back(Node(NodeType::Spread, spread_name));

            comment.clear();
            if (newlineOrComment(&comment))
                args->list().back().attachCommentAfter(comment);

            if (std::ranges::find(names, spread_name) != names.end())
            {
                backtrack(pos);
                errorWithNextToken(fmt::format("Argument names must be unique, can not reuse `{}'", spread_name));
            }
        }

        if (!accept(IsChar(')')))
            return std::nullopt;
        comment.clear();
        if (newlineOrComment(&comment))
        {
            if (args->list().empty())
                args->attachCommentAfter(comment);
            else
                args->list().back().attachCommentAfter(comment);
        }

        return positioned(args, filepos);
    }

    std::optional<Node> Parser::macro(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::Macro };

        auto context = generateErrorContext("(");
        if (!accept(IsChar('(')))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        if (!oneOf({ "macro" }))
            return std::nullopt;
        newlineOrComment(&comment);
        leaf->attachNearestCommentBefore(comment);

        std::string symbol_name;
        if (!name(&symbol_name))
            errorWithNextToken("Expected a symbol to declare a macro");
        comment.clear();
        newlineOrComment(&comment);

        leaf->push_back(Node(NodeType::Symbol, symbol_name).attachNearestCommentBefore(comment));

        const auto position = getCount();
        const auto args_file_pos = getCursor();
        if (const auto args = macroArgs(args_file_pos); args.has_value())
            leaf->push_back(args.value());
        else
        {
            // if we couldn't parse arguments, then we have a value
            backtrack(position);

            ++m_allow_macro_behavior;
            const auto value = nodeOrValue();
            --m_allow_macro_behavior;

            if (value.has_value())
                leaf->push_back(value.value());
            else
                errorWithNextToken(fmt::format("Expected an argument list, atom or node while defining macro `{}'", symbol_name));

            comment.clear();
            if (newlineOrComment(&comment))
                leaf->list().back().attachCommentAfter(comment);
            expectSuffixOrError(')', fmt::format("to close macro `{}'", symbol_name), context);
            return positioned(leaf, filepos);
        }

        ++m_allow_macro_behavior;
        const auto value = nodeOrValue();
        --m_allow_macro_behavior;

        if (value.has_value())
            leaf->push_back(value.value());
        else if (leaf->list().size() == 2)  // the argument list is actually a function call and it's okay
        {
            comment.clear();
            if (newlineOrComment(&comment))
                leaf->list().back().attachCommentAfter(comment);

            expectSuffixOrError(')', fmt::format("to close macro `{}'", symbol_name), context);
            return positioned(leaf, filepos);
        }
        else
        {
            backtrack(position);
            errorWithNextToken(fmt::format("Expected a value while defining macro `{}'", symbol_name), context);
        }

        comment.clear();
        if (newlineOrComment(&comment))
            leaf->list().back().attachCommentAfter(comment);

        expectSuffixOrError(')', fmt::format("to close macro `{}'", symbol_name), context);
        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::functionCall(const FilePosition filepos)
    {
        auto context = generateErrorContext("(");
        if (!accept(IsChar('(')))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        const auto func_name_pos = getCursor();
        std::optional<Node> func;
        if (auto sym_or_field = anyAtomOf({ NodeType::Symbol, NodeType::Field }); sym_or_field.has_value())
            func = sym_or_field->attachNearestCommentBefore(comment);
        else if (auto nested = node(); nested.has_value())
            func = nested->attachNearestCommentBefore(comment);
        else
            return std::nullopt;
        comment.clear();
        newlineOrComment(&comment);

        std::optional<Node> leaf { NodeType::List };
        leaf->push_back(positioned(func.value(), func_name_pos));

        while (!isEOF())
        {
            if (auto arg = nodeOrValue(); arg.has_value())
            {
                leaf->push_back(arg.value().attachNearestCommentBefore(comment));
                comment.clear();
                newlineOrComment(&comment);
            }
            else
                break;
        }

        leaf->list().back().attachCommentAfter(comment);

        comment.clear();
        if (newlineOrComment(&comment))
            leaf->list().back().attachCommentAfter(comment);

        expectSuffixOrError(')', fmt::format("in function call to `{}'", func.value().repr()), context);
        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::list(const FilePosition filepos)
    {
        std::optional<Node> leaf { NodeType::List };

        auto context = generateErrorContext("[");
        if (!accept(IsChar('[')))
            return std::nullopt;
        leaf->setAltSyntax(true);
        leaf->push_back(Node(NodeType::Symbol, "list"));

        std::string comment;
        newlineOrComment(&comment);
        leaf->attachNearestCommentBefore(comment);

        comment.clear();
        while (!isEOF())
        {
            if (auto value = nodeOrValue(); value.has_value())
            {
                leaf->push_back(value.value().attachNearestCommentBefore(comment));
                comment.clear();
                newlineOrComment(&comment);
            }
            else
                break;
        }
        leaf->list().back().attachCommentAfter(comment);

        expectSuffixOrError(']', "to end list definition", context);
        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::number(const FilePosition filepos)
    {
        const long pos = getCount();

        std::string res;
        if (signedNumber(&res))
        {
            double output;
            if (Utils::isDouble(res, &output))
                return positioned(Node(output), filepos);

            // fixme: find a better way to send an error here
            backtrack(pos);
            error("Is not a valid number", res);
        }
        return std::nullopt;
    }

    std::optional<Node> Parser::string(const FilePosition filepos)
    {
        std::string res;
        if (accept(IsChar('"')))
        {
            while (true)
            {
                if (accept(IsChar('\\')))
                {
                    if (!m_interpret)
                        res += '\\';

                    if (accept(IsChar('"')))
                        res += '"';
                    else if (accept(IsChar('\\')))
                        res += '\\';
                    else if (accept(IsChar('n')))
                        res += m_interpret ? '\n' : 'n';
                    else if (accept(IsChar('t')))
                        res += m_interpret ? '\t' : 't';
                    else if (accept(IsChar('v')))
                        res += m_interpret ? '\v' : 'v';
                    else if (accept(IsChar('r')))
                        res += m_interpret ? '\r' : 'r';
                    else if (accept(IsChar('a')))
                        res += m_interpret ? '\a' : 'a';
                    else if (accept(IsChar('b')))
                        res += m_interpret ? '\b' : 'b';
                    else if (accept(IsChar('f')))
                        res += m_interpret ? '\f' : 'f';
                    else if (accept(IsChar('u')))
                    {
                        std::string seq;
                        if (hexNumber(4, &seq))
                        {
                            if (m_interpret)
                            {
                                char utf8_str[5];
                                utf8::decode(seq.c_str(), utf8_str);
                                if (*utf8_str == '\0')
                                    error("Invalid escape sequence", "\\u" + seq);
                                res += utf8_str;
                            }
                            else
                                res += "u" + seq;
                        }
                        else
                            error("Invalid escape sequence", "\\u");
                    }
                    else if (accept(IsChar('U')))
                    {
                        std::string seq;
                        if (hexNumber(8, &seq))
                        {
                            if (m_interpret)
                            {
                                std::size_t begin = 0;
                                for (; seq[begin] == '0'; ++begin)
                                    ;
                                char utf8_str[5];
                                utf8::decode(seq.c_str() + begin, utf8_str);
                                if (*utf8_str == '\0')
                                    error("Invalid escape sequence", "\\U" + seq);
                                res += utf8_str;
                            }
                            else
                                res += "U" + seq;
                        }
                        else
                            error("Invalid escape sequence", "\\U");
                    }
                    else
                    {
                        backtrack(getCount() - 1);
                        error("Unknown escape sequence", "\\");
                    }
                }
                else
                    accept(IsNot(IsEither(IsChar('\\'), IsChar('"'))), &res);

                if (accept(IsChar('"')))
                    break;
                if (isEOF())
                    expectSuffixOrError('"', "after string");
            }

            return positioned(Node(NodeType::String, res), filepos);
        }
        return std::nullopt;
    }

    std::optional<Node> Parser::field(const FilePosition filepos)
    {
        std::string sym;
        if (!name(&sym))
            return std::nullopt;

        std::optional<Node> leaf { Node(NodeType::Field) };
        leaf->push_back(Node(NodeType::Symbol, sym));

        while (true)
        {
            if (leaf->list().size() == 1 && !accept(IsChar('.')))  // Symbol:abc
                return std::nullopt;

            if (leaf->list().size() > 1 && !accept(IsChar('.')))
                break;

            const auto filepos_inner = getCursor();
            std::string res;
            if (!name(&res))
                errorWithNextToken("Expected a field name: <symbol>.<field>");
            leaf->push_back(positioned(Node(NodeType::Symbol, res), filepos_inner));
        }

        return positioned(leaf, filepos);
    }

    std::optional<Node> Parser::symbol(const FilePosition filepos)
    {
        std::string res;
        if (!name(&res))
            return std::nullopt;
        return positioned(Node(NodeType::Symbol, res), filepos);
    }

    std::optional<Node> Parser::spread(const FilePosition filepos)
    {
        std::string res;
        if (sequence("..."))
        {
            if (!name(&res))
                errorWithNextToken("Expected a name for the variadic");
            return positioned(Node(NodeType::Spread, res), filepos);
        }
        return std::nullopt;
    }

    std::optional<Node> Parser::nil(const FilePosition filepos)
    {
        if (!accept(IsChar('(')))
            return std::nullopt;

        std::string comment;
        newlineOrComment(&comment);
        if (!accept(IsChar(')')))
            return std::nullopt;

        if (m_interpret)
            return positioned(Node(NodeType::Symbol, "nil").attachNearestCommentBefore(comment), filepos);
        return positioned(Node(NodeType::List).attachNearestCommentBefore(comment), filepos);
    }

    std::optional<Node> Parser::atom()
    {
        const auto pos = getCount();
        const auto filepos = getCursor();

        if (auto res = Parser::number(filepos); res.has_value())
            return res;
        backtrack(pos);

        if (auto res = Parser::string(filepos); res.has_value())
            return res;
        backtrack(pos);

        if (auto res = Parser::spread(filepos); m_allow_macro_behavior > 0 && res.has_value())
            return res;
        backtrack(pos);

        if (auto res = Parser::field(filepos); res.has_value())
            return res;
        backtrack(pos);

        if (auto res = Parser::symbol(filepos); res.has_value())
            return res;
        backtrack(pos);

        if (auto res = Parser::nil(filepos); res.has_value())
            return res;
        backtrack(pos);

        return std::nullopt;
    }

    std::optional<Node> Parser::anyAtomOf(const std::initializer_list<NodeType> types)
    {
        if (auto value = atom(); value.has_value())
        {
            for (const auto type : types)
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
        if (auto sub_node = node(); sub_node.has_value())
            return sub_node;

        return std::nullopt;
    }

    std::optional<Node> Parser::wrapped(std::optional<Node> (Parser::*parser)(FilePosition), const std::string& name)
    {
        const auto cursor = getCursor();
        auto context = generateErrorContext("(");
        if (!prefix('('))
            return std::nullopt;
        std::string comment;
        newlineOrComment(&comment);

        if (auto result = (this->*parser)(cursor); result.has_value())
        {
            result->attachNearestCommentBefore(result->comment() + comment);

            comment.clear();
            if (newlineOrComment(&comment))
                result.value().attachCommentAfter(comment);

            expectSuffixOrError(')', "after " + name, context);

            comment.clear();
            if (spaceComment(&comment))
                result.value().attachCommentAfter(comment);

            return result;
        }

        return std::nullopt;
    }
}
