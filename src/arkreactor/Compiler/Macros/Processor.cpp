#include <Ark/Compiler/Macros/Processor.hpp>

#include <utility>
#include <algorithm>
#include <cassert>
#include <ranges>
#include <sstream>
#include <fmt/core.h>

#include <Ark/Constants.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Compiler/Macros/Executor.hpp>
#include <Ark/Compiler/Macros/Executors/Symbol.hpp>
#include <Ark/Compiler/Macros/Executors/Function.hpp>
#include <Ark/Compiler/Macros/Executors/Conditional.hpp>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(const unsigned debug) noexcept :
        Pass("MacroProcessor", debug)
    {
        // create executors pipeline
        m_executors = { { std::make_shared<SymbolExecutor>(this),
                          std::make_shared<ConditionalExecutor>(this),
                          std::make_shared<FunctionExecutor>(this) } };
    }

    void MacroProcessor::process(const Node& ast)
    {
        m_logger.debug("Processing macros...");

        // to be able to modify it
        m_ast = ast;
        processNode(m_ast, 0);

        m_logger.trace("AST after processing macros");
        if (m_logger.shouldTrace())
            m_ast.debugPrint(std::cout) << '\n';
    }

    const Node& MacroProcessor::ast() const noexcept
    {
        return m_ast;
    }

    void MacroProcessor::registerMacro(Node& node)
    {
        // a macro needs at least 2 nodes, name + value is the minimal form
        // this is guaranted by the parser
        assert(node.constList().size() >= 2 && "Invalid macro, missing value");

        const Node& first_node = node.list()[0];

        // ($ name value)
        if (node.constList().size() == 2)
        {
            assert(first_node.nodeType() == NodeType::Symbol && "Can not define a macro without a symbol");
            m_macros.back().add(first_node.string(), node);
        }
        // ($ name (args) body)
        else if (node.constList().size() == 3 && first_node.nodeType() == NodeType::Symbol)
        {
            assert(node.list()[1].nodeType() == NodeType::List && "Invalid macro argument's list");
            m_macros.back().add(first_node.string(), node);
        }
    }

    void MacroProcessor::registerFuncDef(const Node& node)
    {
        if (node.nodeType() == NodeType::List && node.constList().size() == 3 && node.constList()[0].nodeType() == NodeType::Keyword)
        {
            Keyword kw = node.constList()[0].keyword();
            // checking for function definition, which can occur only inside an assignment node
            if (kw != Keyword::Let && kw != Keyword::Mut && kw != Keyword::Set)
                return;

            const Node inner = node.constList()[2];
            if (inner.nodeType() != NodeType::List)
                return;

            if (!inner.constList().empty() && inner.constList()[0].nodeType() == NodeType::Keyword && inner.constList()[0].keyword() == Keyword::Fun)
            {
                const Node symbol = node.constList()[1];
                if (symbol.nodeType() == NodeType::Symbol)
                    m_defined_functions.emplace(symbol.string(), inner.constList()[1]);
                else
                    throwMacroProcessingError(fmt::format("Can not use a {} to define a variable", typeToString(symbol)), symbol);
            }
        }
    }

    void MacroProcessor::processNode(Node& node, unsigned depth)
    {
        if (depth >= MaxMacroProcessingDepth)
            throwMacroProcessingError(
                fmt::format(
                    "Max recursion depth reached ({}). You most likely have a badly defined recursive macro calling itself without a proper exit condition",
                    MaxMacroProcessingDepth),
                node);

        if (node.nodeType() == NodeType::List)
        {
            bool has_created = false;
            // recursive call
            std::size_t i = 0;
            while (i < node.list().size())
            {
                if (node.list()[i].nodeType() == NodeType::Macro)
                {
                    // create a scope only if needed
                    if ((!m_macros.empty() && !m_macros.back().empty() && m_macros.back().depth() < depth) || !has_created)
                    {
                        has_created = true;
                        m_macros.emplace_back(depth);
                    }

                    const bool had = hadBegin(node.list()[i]);

                    // register the macro we encountered
                    registerMacro(node.list()[i]);
                    recurApply(node.list()[i]);

                    // if we now have a surrounding (begin ...) and didn't have one before, remove it
                    if (hadBegin(node.list()[i]) && !had)
                        removeBegin(node, i);
                    // if there is an unused node or a leftover macro need, we need to get rid of it in the final ast
                    else if (node.list()[i].nodeType() == NodeType::Macro || node.list()[i].nodeType() == NodeType::Unused)
                        node.list().erase(node.constList().begin() + static_cast<std::vector<Node>::difference_type>(i));
                }
                else  // running on non-macros
                {
                    bool added_begin = false;

                    const bool had = hadBegin(node.list()[i]);
                    applyMacro(node.list()[i], 0);
                    recurApply(node.list()[i]);

                    if (hadBegin(node.list()[i]) && !had)
                        added_begin = true;
                    else if (node.list()[i].nodeType() == NodeType::Unused)
                        node.list().erase(node.constList().begin() + static_cast<std::vector<Node>::difference_type>(i));

                    if (node.nodeType() == NodeType::List && i < node.constList().size())
                    {
                        processNode(node.list()[i], depth + 1);
                        // needed if we created a function node from a macro
                        registerFuncDef(node.list()[i]);
                    }

                    // remove begins in macros
                    if (added_begin && i < node.constList().size())
                        removeBegin(node, i);

                    // go forward only if it isn't a macro, because we delete macros
                    // while running on the AST
                    ++i;
                }
            }

            // delete a scope only if needed
            if (!m_macros.empty() && m_macros.back().depth() == depth)
                m_macros.pop_back();
        }
    }

    bool MacroProcessor::applyMacro(Node& node, const unsigned depth)
    {
        if (depth > MaxMacroProcessingDepth)
            throwMacroProcessingError(
                fmt::format(
                    "Max macro processing depth reached ({}). You may have a macro trying to evaluate itself, try splitting your code in multiple nodes.",
                    MaxMacroProcessingDepth),
                node);

        for (const auto& executor : m_executors)
        {
            if (executor->canHandle(node))
            {
                if (executor->applyMacro(node, depth))
                    return true;
            }
        }
        return false;
    }

    void MacroProcessor::unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, const std::size_t index, const std::size_t unify_depth)
    {
        if (unify_depth > MaxMacroUnificationDepth)
            throwMacroProcessingError(
                fmt::format(
                    "Max macro unification depth reached ({}). You may have a macro trying to evaluate itself, try splitting your code in multiple nodes.",
                    MaxMacroUnificationDepth),
                *parent);

        if (target.nodeType() == NodeType::Symbol)
        {
            if (const auto p = map.find(target.string()); p != map.end())
                target = p->second;
        }
        else if (target.isListLike())
        {
            for (std::size_t i = 0; i < target.list().size(); ++i)
                unify(map, target.list()[i], &target, i, unify_depth + 1);
        }
        else if (target.nodeType() == NodeType::Spread)
        {
            Node sub_node = target;
            sub_node.setNodeType(NodeType::Symbol);
            unify(map, sub_node, parent, 0, unify_depth + 1);

            if (sub_node.nodeType() != NodeType::List)
                throwMacroProcessingError(fmt::format("Can not unify a {} to a Spread", typeToString(sub_node)), sub_node);

            for (std::size_t i = 1, end = sub_node.list().size(); i < end; ++i)
                parent->list().insert(
                    parent->list().begin() + static_cast<std::vector<Node>::difference_type>(index + i),
                    sub_node.list()[i]);
            parent->list().erase(parent->list().begin() + static_cast<std::vector<Node>::difference_type>(index));  // remove the spread
        }
    }

    void MacroProcessor::setWithFileAttributes(const Node origin, Node& output, const Node& macro)
    {
        output = macro;
        output.setFilename(origin.filename());
        output.setPos(origin.line(), origin.col());
    }

    void MacroProcessor::checkMacroArgCount(const Node& node, std::size_t expected, const std::string& name, const std::string& kind)
    {
        const std::size_t argcount = node.constList().size();
        if (argcount != expected + 1)
            throwMacroProcessingError(
                fmt::format(
                    "Interpreting a `{}'{} with {} arguments, expected {}.",
                    name,
                    kind.empty() ? kind : " " + kind,
                    argcount,
                    expected),
                node);
    }

    Node MacroProcessor::evaluate(Node& node, const unsigned depth, const bool is_not_body)
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            const Node* macro = findNearestMacro(node.string());
            if (macro != nullptr && macro->constList().size() == 2)
                return macro->constList()[1];
            return node;
        }
        if (node.nodeType() == NodeType::List && !node.constList().empty() && node.list()[0].nodeType() == NodeType::Symbol)
        {
            const std::string& name = node.list()[0].string();
            const std::size_t argcount = node.list().size() - 1;

            if (const Node* macro = findNearestMacro(name); macro != nullptr)
            {
                applyMacro(node.list()[0], depth + 1);
                if (node.list()[0].nodeType() == NodeType::Unused)
                    node.list().erase(node.constList().begin());
            }
            else if (name == "=" && is_not_body)
            {
                checkMacroArgCount(node, 2, "=", "condition");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one == two) ? getTrueNode() : getFalseNode();
            }
            else if (name == "!=" && is_not_body)
            {
                checkMacroArgCount(node, 2, "!=", "condition");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one != two) ? getTrueNode() : getFalseNode();
            }
            else if (name == "<" && is_not_body)
            {
                checkMacroArgCount(node, 2, "<", "condition");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one < two) ? getTrueNode() : getFalseNode();
            }
            else if (name == ">" && is_not_body)
            {
                checkMacroArgCount(node, 2, ">", "condition");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return !(one < two) && (one != two) ? getTrueNode() : getFalseNode();
            }
            else if (name == "<=" && is_not_body)
            {
                checkMacroArgCount(node, 2, "<=", "condition");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return one < two || one == two ? getTrueNode() : getFalseNode();
            }
            else if (name == ">=" && is_not_body)
            {
                checkMacroArgCount(node, 2, ">=", "condition");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return !(one < two) ? getTrueNode() : getFalseNode();
            }
            else if (name == "+" && is_not_body)
            {
                checkMacroArgCount(node, 2, "+", "operator");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one.nodeType() == two.nodeType() && two.nodeType() == NodeType::Number) ? Node(one.number() + two.number()) : node;
            }
            else if (name == "-" && is_not_body)
            {
                checkMacroArgCount(node, 2, "-", "operator");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one.nodeType() == two.nodeType() && two.nodeType() == NodeType::Number) ? Node(one.number() - two.number()) : node;
            }
            else if (name == "*" && is_not_body)
            {
                checkMacroArgCount(node, 2, "*", "operator");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one.nodeType() == two.nodeType() && two.nodeType() == NodeType::Number) ? Node(one.number() * two.number()) : node;
            }
            else if (name == "/" && is_not_body)
            {
                checkMacroArgCount(node, 2, "/", "operator");
                const Node one = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node two = evaluate(node.list()[2], depth + 1, is_not_body);
                return (one.nodeType() == two.nodeType() && two.nodeType() == NodeType::Number) ? Node(one.number() / two.number()) : node;
            }
            else if (name == "not" && is_not_body)
            {
                checkMacroArgCount(node, 1, "not", "condition");
                return (!isTruthy(evaluate(node.list()[1], depth + 1, is_not_body))) ? getTrueNode() : getFalseNode();
            }
            else if (name == Language::And && is_not_body)
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError(fmt::format("Interpreting a `{}' chain with {} arguments, expected at least 2.", Language::And, argcount), node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (!isTruthy(evaluate(node.list()[i], depth + 1, is_not_body)))
                        return getFalseNode();
                }
                return getTrueNode();
            }
            else if (name == Language::Or && is_not_body)
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError(fmt::format("Interpreting an `{}' chain with {} arguments, expected at least 2.", Language::Or, argcount), node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (isTruthy(evaluate(node.list()[i], depth + 1, is_not_body)))
                        return getTrueNode();
                }
                return getFalseNode();
            }
            else if (name == "len")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `len' inside a macro, got {} arguments, expected 1", argcount), node);
                if (Node& lst = node.list()[1]; lst.nodeType() == NodeType::List)  // only apply len at compile time if we can
                {
                    if (isConstEval(lst))
                    {
                        if (!lst.list().empty() && lst.list()[0] == getListNode())
                            setWithFileAttributes(node, node, Node(static_cast<long>(lst.list().size()) - 1));
                        else
                            setWithFileAttributes(node, node, Node(static_cast<long>(lst.list().size())));
                    }
                }
            }
            else if (name == "empty?")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `empty?' inside a macro, got {} arguments, expected 1", argcount), node);
                if (Node& lst = node.list()[1]; lst.nodeType() == NodeType::List && isConstEval(lst))
                {
                    // only apply len at compile time if we can
                    if (!lst.list().empty() && lst.list()[0] == getListNode())
                        setWithFileAttributes(node, node, lst.list().size() - 1 == 0 ? getTrueNode() : getFalseNode());
                    else
                        setWithFileAttributes(node, node, lst.list().empty() ? getTrueNode() : getFalseNode());
                }
                else if (lst == getNilNode())
                    setWithFileAttributes(node, node, getTrueNode());
            }
            else if (name == "@")
            {
                checkMacroArgCount(node, 2, "@");

                Node sublist = evaluate(node.list()[1], depth + 1, is_not_body);
                const Node idx = evaluate(node.list()[2], depth + 1, is_not_body);

                if (sublist.nodeType() == NodeType::List && idx.nodeType() == NodeType::Number)
                {
                    const std::size_t size = sublist.list().size();
                    std::size_t real_size = size;
                    long num_idx = static_cast<long>(idx.number());

                    // if the first node is the function call to "list", don't count it
                    if (size > 0 && sublist.list()[0] == getListNode())
                    {
                        real_size--;
                        if (num_idx >= 0)
                            ++num_idx;
                    }

                    Node output;
                    if (num_idx >= 0 && std::cmp_less(num_idx, size))
                        output = sublist.list()[static_cast<std::size_t>(num_idx)];
                    else if (const auto c = static_cast<long>(size) + num_idx; num_idx < 0 && std::cmp_less(c, size) && c >= 0)
                        output = sublist.list()[static_cast<std::size_t>(c)];
                    else
                        throwMacroProcessingError(fmt::format("Index ({}) out of range (list size: {})", num_idx, real_size), node);

                    output.setFilename(node.filename());
                    output.setPos(node.line(), node.col());
                    return output;
                }
            }
            else if (name == "head")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `head' inside a macro, got {} arguments, expected 1", argcount), node);
                if (node.list()[1].nodeType() == NodeType::List)
                {
                    Node& sublist = node.list()[1];
                    if (!sublist.constList().empty() && sublist.constList()[0] == getListNode())
                    {
                        if (sublist.constList().size() > 1)
                        {
                            const Node sublistCopy = sublist.constList()[1];
                            setWithFileAttributes(node, node, sublistCopy);
                        }
                        else
                            setWithFileAttributes(node, node, getNilNode());
                    }
                    else if (!sublist.list().empty())
                        setWithFileAttributes(node, node, sublist.constList()[0]);
                    else
                        setWithFileAttributes(node, node, getNilNode());
                }
            }
            else if (name == "tail")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `tail' inside a macro, got {} arguments, expected 1", argcount), node);
                if (node.list()[1].nodeType() == NodeType::List)
                {
                    Node sublist = node.list()[1];
                    if (!sublist.list().empty() && sublist.list()[0] == getListNode())
                    {
                        if (sublist.list().size() > 1)
                        {
                            sublist.list().erase(sublist.constList().begin() + 1);
                            setWithFileAttributes(node, node, sublist);
                        }
                        else
                        {
                            setWithFileAttributes(node, node, Node(NodeType::List));
                            node.push_back(getListNode());
                        }
                    }
                    else if (!sublist.list().empty())
                    {
                        sublist.list().erase(sublist.constList().begin());
                        sublist.list().insert(sublist.list().begin(), getListNode());
                        setWithFileAttributes(node, node, sublist);
                    }
                    else
                    {
                        setWithFileAttributes(node, node, Node(NodeType::List));
                        node.push_back(getListNode());
                    }
                }
            }
            else if (name == Language::Symcat)
            {
                if (node.list().size() <= 2)
                    throwMacroProcessingError(fmt::format("When expanding `{}', expected at least 2 arguments, got {} arguments", Language::Symcat, argcount), node);
                if (node.list()[1].nodeType() != NodeType::Symbol)
                    throwMacroProcessingError(fmt::format("When expanding `{}', expected the first argument to be a Symbol, got a {}", Language::Symcat, typeToString(node.list()[1])), node);

                std::string sym = node.list()[1].string();

                for (std::size_t i = 2, end = node.list().size(); i < end; ++i)
                {
                    const Node ev = evaluate(node.list()[i], depth + 1, /* is_not_body */ true);

                    switch (ev.nodeType())
                    {
                        case NodeType::Number:
                            // we don't want '.' in identifiers
                            sym += std::to_string(static_cast<long int>(ev.number()));
                            break;

                        case NodeType::String:
                        case NodeType::Symbol:
                            sym += ev.string();
                            break;

                        default:
                            throwMacroProcessingError(fmt::format("When expanding `{}', expected either a Number, String or Symbol, got a {}", Language::Symcat, typeToString(ev)), ev);
                    }
                }

                node.setNodeType(NodeType::Symbol);
                node.setString(sym);
            }
            else if (name == Language::Argcount)
            {
                const Node sym = node.constList()[1];
                if (sym.nodeType() == NodeType::Symbol)
                {
                    if (const auto maybe_func = lookupDefinedFunction(sym.string()); maybe_func.has_value())
                        setWithFileAttributes(node, node, Node(static_cast<long>(maybe_func->constList().size())));
                    else
                        throwMacroProcessingError(fmt::format("When expanding `{}', expected a known function name, got unbound variable {}", Language::Argcount, sym.string()), sym);
                }
                else if (sym.nodeType() == NodeType::List && sym.constList().size() == 3 && sym.constList()[0].nodeType() == NodeType::Keyword && sym.constList()[0].keyword() == Keyword::Fun)
                    setWithFileAttributes(node, node, Node(static_cast<long>(sym.constList()[1].constList().size())));
                else
                    throwMacroProcessingError(fmt::format("When trying to apply `{}', got a {} instead of a Symbol or Function", Language::Argcount, typeToString(sym)), sym);
            }
            else if (name == Language::Repr)
            {
                const Node ast = node.constList()[1];
                setWithFileAttributes(node, node, Node(NodeType::String, ast.repr()));
            }
            else if (name == Language::Paste)
            {
                if (node.list().size() != 2)
                    throwMacroProcessingError(fmt::format("When expanding `{}', expected one argument, got {} arguments", Language::Paste, argcount), node);
                return node.constList()[1];
            }
            else if (name == Language::Undef)
            {
                if (node.list().size() != 2)
                    throwMacroProcessingError(fmt::format("When expanding `{}', expected one argument, got {} arguments", Language::Undef, argcount), node);

                const Node sym = node.constList()[1];
                if (sym.nodeType() == NodeType::Symbol)
                {
                    deleteNearestMacro(sym.string());
                    node.setNodeType(NodeType::Unused);
                    return node;
                }

                throwMacroProcessingError(
                    fmt::format(
                        "When expanding `{}', got a {}. Can not un-define a macro without a valid name",
                        Language::Undef, typeToString(sym)),
                    sym);
            }
        }

        if (node.nodeType() == NodeType::List && !node.constList().empty())
        {
            for (auto& child : node.list())
                setWithFileAttributes(child, child, evaluate(child, depth + 1, is_not_body));
        }

        if (node.nodeType() == NodeType::Spread)
            throwMacroProcessingError(fmt::format("Found an unevaluated spread: `{}'", node.string()), node);

        return node;
    }

    bool MacroProcessor::isTruthy(const Node& node)
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            if (node.string() == "true")
                return true;
            if (node.string() == "false" || node.string() == "nil")
                return false;
        }
        else if ((node.nodeType() == NodeType::Number && node.number() != 0.0) || (node.nodeType() == NodeType::String && !node.string().empty()))
            return true;
        else if (node.nodeType() == NodeType::Spread)
            throwMacroProcessingError("Can not determine the truth value of a spreaded symbol", node);
        return false;
    }

    std::optional<Node> MacroProcessor::lookupDefinedFunction(const std::string& name) const
    {
        if (m_defined_functions.contains(name))
            return m_defined_functions.at(name);
        return std::nullopt;
    }

    const Node* MacroProcessor::findNearestMacro(const std::string& name) const
    {
        if (m_macros.empty())
            return nullptr;

        for (const auto& m_macro : std::ranges::reverse_view(m_macros))
        {
            if (const auto res = m_macro.has(name); res != nullptr)
                return res;
        }
        return nullptr;
    }

    void MacroProcessor::deleteNearestMacro(const std::string& name)
    {
        if (m_macros.empty())
            return;

        for (auto& m_macro : std::ranges::reverse_view(m_macros))
        {
            if (m_macro.remove(name))
            {
                // stop right here because we found one matching macro
                return;
            }
        }
    }

    void MacroProcessor::recurApply(Node& node)
    {
        if (applyMacro(node, 0) && node.isListLike())
        {
            for (auto& child : node.list())
                recurApply(child);
        }
    }

    bool MacroProcessor::hadBegin(const Node& node)
    {
        return node.nodeType() == NodeType::List &&
            !node.constList().empty() &&
            node.constList()[0].nodeType() == NodeType::Keyword &&
            node.constList()[0].keyword() == Keyword::Begin;
    }

    void MacroProcessor::removeBegin(Node& node, std::size_t i)
    {
        if (node.isListLike() && node.list()[i].nodeType() == NodeType::List && !node.list()[i].list().empty())
        {
            Node lst = node.constList()[i];
            Node first = lst.constList()[0];

            if (first.nodeType() == NodeType::Keyword && first.keyword() == Keyword::Begin)
            {
                const std::size_t previous = i;

                for (std::size_t block_idx = 1, end = lst.constList().size(); block_idx < end; ++block_idx)
                    node.list().insert(
                        node.constList().begin() + static_cast<std::vector<Node>::difference_type>(i + block_idx),
                        lst.list()[block_idx]);

                node.list().erase(node.constList().begin() + static_cast<std::vector<Node>::difference_type>(previous));
            }
        }
    }

    bool MacroProcessor::isConstEval(const Node& node) const
    {
        switch (node.nodeType())
        {
            case NodeType::Symbol:
            {
                const auto it = std::ranges::find(Language::operators, node.string());
                const auto it2 = std::ranges::find_if(Builtins::builtins,
                                                      [&node](const std::pair<std::string, Value>& element) -> bool {
                                                          return node.string() == element.first;
                                                      });

                return it != Language::operators.end() ||
                    it2 != Builtins::builtins.end() ||
                    findNearestMacro(node.string()) != nullptr ||
                    node.string() == "list" ||
                    node.string() == "nil";
            }

            case NodeType::List:
                return std::ranges::all_of(node.constList(), [this](const Node& child) {
                    return isConstEval(child);
                });

            case NodeType::Capture:
            case NodeType::Field:
                return false;

            case NodeType::Keyword:
            case NodeType::String:
            case NodeType::Number:
            case NodeType::Macro:
            case NodeType::Spread:
            case NodeType::Unused:
                return true;
        }

        return false;
    }

    void MacroProcessor::throwMacroProcessingError(const std::string& message, const Node& node)
    {
        throw CodeError(message, node.filename(), node.line(), node.col(), node.repr());
    }
}
