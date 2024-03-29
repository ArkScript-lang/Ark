#include <Ark/Compiler/Macros/Processor.hpp>

#include <algorithm>
#include <utility>
#include <sstream>
#include <fmt/core.h>

#include <Ark/Exceptions.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Compiler/Macros/Executors/Symbol.hpp>
#include <Ark/Compiler/Macros/Executors/Function.hpp>
#include <Ark/Compiler/Macros/Executors/Conditional.hpp>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(unsigned debug) noexcept :
        m_debug(debug)
    {
        // create executors pipeline
        m_executor_pipeline = MacroExecutorPipeline(
            { std::make_shared<SymbolExecutor>(this),
              std::make_shared<ConditionalExecutor>(this),
              std::make_shared<FunctionExecutor>(this) });

        m_predefined_macros = {
            "symcat",
            "argcount",
            "$repr"  // TODO: unify predefined macro names (update documentation and examples and tests)
        };
    }

    void MacroProcessor::process(const Node& ast)
    {
        if (m_debug >= 2)
            std::cout << "Processing macros...\n";

        // to be able to modify it
        m_ast = ast;
        processNode(m_ast, 0);

        if (m_debug >= 3)
        {
            std::cout << "(MacroProcessor) AST after processing macros\n";
            std::cout << m_ast << '\n';
        }
    }

    const Node& MacroProcessor::ast() const noexcept
    {
        return m_ast;
    }

    void MacroProcessor::registerMacro(Node& node)
    {
        // a macro needs at least 2 nodes, name + value is the minimal form
        if (node.constList().size() < 2)
            throwMacroProcessingError("Invalid macro, missing value", node);

        const Node& first_node = node.list()[0];
        const Node& second_node = node.list()[1];

        // ($ name value)
        if (node.constList().size() == 2)
        {
            if (first_node.nodeType() == NodeType::Symbol)
            {
                if (first_node.string() != "$undef")
                    m_macros.back().add(first_node.string(), node);
                else if (second_node.nodeType() == NodeType::Symbol)  // un-define a macro
                    deleteNearestMacro(second_node.string());
                else  // used undef on a non-symbol
                    throwMacroProcessingError("Can not un-define a macro without a name", second_node);
            }
            else
                throwMacroProcessingError("Can not define a macro without a symbol", first_node);
        }
        // ($ name (args) body)
        else if (node.constList().size() == 3 && first_node.nodeType() == NodeType::Symbol)
        {
            if (second_node.nodeType() != NodeType::List)
                throwMacroProcessingError("Invalid macro argument's list", second_node);
            else
            {
                bool had_spread = false;
                for (const Node& n : second_node.constList())
                {
                    if (n.nodeType() != NodeType::Symbol && n.nodeType() != NodeType::Spread)
                        throwMacroProcessingError("Invalid macro argument's list, expected symbols", n);
                    else if (n.nodeType() == NodeType::Spread)
                    {
                        if (had_spread)
                            throwMacroProcessingError("Invalid macro, multiple spread detected in argument list but only one is allowed", n);
                        had_spread = true;
                    }
                    else if (had_spread && n.nodeType() == NodeType::Symbol)
                        throwMacroProcessingError(fmt::format("Invalid macro, a spread should mark the end of an argument list, but found another argument: {}", n.string()), n);
                }
                m_macros.back().add(first_node.string(), node);
            }
        }
    }

    void MacroProcessor::registerFuncDef(Node& node)
    {
        if (node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Keyword)
        {
            Keyword kw = node.constList()[0].keyword();
            // checking for function definition, which can occur only inside an assignment node
            if (kw != Keyword::Let && kw != Keyword::Mut && kw != Keyword::Set)
                return;

            const Node& inner = node.constList()[2];
            if (inner.nodeType() != NodeType::List)
                return;

            if (!inner.constList().empty() && inner.constList()[0].nodeType() == NodeType::Keyword && inner.constList()[0].keyword() == Keyword::Fun)
                m_defined_functions[node.constList()[1].string()] = inner.constList()[1];
        }
    }

    void MacroProcessor::processNode(Node& node, unsigned depth)
    {
        bool has_created = false;

        if (node.nodeType() == NodeType::List)
        {
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

                    bool had = hadBegin(node.list()[i]);

                    registerMacro(node.list()[i]);
                    if (node.list()[i].nodeType() == NodeType::Macro)
                        recurApply(node.list()[i]);

                    if (hadBegin(node.list()[i]) && !had)
                        removeBegin(node, i);
                    else if (node.list()[i].nodeType() == NodeType::Macro || node.list()[i].nodeType() == NodeType::Unused)
                        node.list().erase(node.constList().begin() + i);
                }
                else  // running on non-macros
                {
                    bool added_begin = false;

                    bool had = hadBegin(node.list()[i]);
                    applyMacro(node.list()[i]);
                    recurApply(node.list()[i]);

                    if (hadBegin(node.list()[i]) && !had)
                        added_begin = true;
                    else if (node.list()[i].nodeType() == NodeType::Unused)
                        node.list().erase(node.constList().begin() + i);

                    if (node.nodeType() == NodeType::List)
                    {
                        processNode(node.list()[i], depth + 1);
                        // needed if we created a function node from a macro
                        registerFuncDef(node.list()[i]);
                    }

                    // remove begins in macros
                    if (added_begin)
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

    bool MacroProcessor::applyMacro(Node& node)
    {
        return m_executor_pipeline.applyMacro(node);
    }

    void MacroProcessor::unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, std::size_t index)
    {
        if (target.nodeType() == NodeType::Symbol)
        {
            if (auto p = map.find(target.string()); p != map.end())
                target = p->second;
        }
        else if (target.isListLike())
        {
            for (std::size_t i = 0, end = target.list().size(); i < end; ++i)
                unify(map, target.list()[i], &target, i);
        }
        else if (target.nodeType() == NodeType::Spread)
        {
            Node sub_node = target;
            sub_node.setNodeType(NodeType::Symbol);
            unify(map, sub_node, parent);

            if (sub_node.nodeType() != NodeType::List)
                throwMacroProcessingError(fmt::format("Can not unify a {} to a Spread", typeToString(sub_node)), sub_node);

            for (std::size_t i = 1, end = sub_node.list().size(); i < end; ++i)
                parent->list().insert(parent->list().begin() + index + i, sub_node.list()[i]);
            parent->list().erase(parent->list().begin() + index);  // remove the spread
        }
    }

    Node MacroProcessor::evaluate(Node& node, bool is_not_body)
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            const Node* macro = findNearestMacro(node.string());
            if (macro != nullptr && macro->constList().size() == 2)
                return macro->constList()[1];
            else
                return node;
        }
        else if (node.nodeType() == NodeType::List && node.constList().size() > 1 && node.list()[0].nodeType() == NodeType::Symbol)
        {
#define GEN_NOT_BODY(str_name, error_handler, ret)        \
    else if (name == str_name && is_not_body)             \
    {                                                     \
        if (node.list().size() != 3) error_handler;       \
        Node one = evaluate(node.list()[1], is_not_body), \
             two = evaluate(node.list()[2], is_not_body); \
        return ret;                                       \
    }

#define GEN_COMPARATOR(str_name, cond) GEN_NOT_BODY(                                                     \
    str_name,                                                                                            \
    throwMacroProcessingError(                                                                           \
        fmt::format("Interpreting a `{}' condition with {} arguments, expected 2.", str_name, argcount), \
        node),                                                                                           \
    (cond) ? getTrueNode() : getFalseNode())

#define GEN_OP(str_name, op) GEN_NOT_BODY(                                                               \
    str_name,                                                                                            \
    throwMacroProcessingError(                                                                           \
        fmt::format("Interpreting a `{}' operation with {} arguments, expected 2.", str_name, argcount), \
        node),                                                                                           \
    (one.nodeType() == two.nodeType() && two.nodeType() == NodeType::Number) ? Node(one.number() op two.number()) : node)

            const std::string& name = node.list()[0].string();
            const std::size_t argcount = node.list().size() - 1;
            if (const Node* macro = findNearestMacro(name); macro != nullptr)
            {
                applyMacro(node.list()[0]);
                if (node.list()[0].nodeType() == NodeType::Unused)
                    node.list().erase(node.constList().begin());
            }
            GEN_COMPARATOR("=", one == two)
            GEN_COMPARATOR("!=", !(one == two))
            GEN_COMPARATOR("<", one < two)
            GEN_COMPARATOR(">", !(one < two) && !(one == two))
            GEN_COMPARATOR("<=", one < two || one == two)
            GEN_COMPARATOR(">=", !(one < two))
            GEN_OP("+", +)
            GEN_OP("-", -)
            GEN_OP("*", *)
            GEN_OP("/", /)
            else if (name == "not" && is_not_body)
            {
                if (node.list().size() != 2)
                    throwMacroProcessingError(fmt::format("Interpreting a `not' condition with {} arguments, expected 1.", argcount), node);

                return (!isTruthy(evaluate(node.list()[1], is_not_body))) ? getTrueNode() : getFalseNode();
            }
            else if (name == "and" && is_not_body)
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError(fmt::format("Interpreting a `and' chain with {} arguments, expected at least 2.", argcount), node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (!isTruthy(evaluate(node.list()[i], is_not_body)))
                        return getFalseNode();
                }
                return getTrueNode();
            }
            else if (name == "or" && is_not_body)
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError(fmt::format("Interpreting an `or' chain with {} arguments, expected at least 2.", argcount), node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (isTruthy(evaluate(node.list()[i], is_not_body)))
                        return getTrueNode();
                }
                return getFalseNode();
            }
            else if (name == "len")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `len' inside a macro, got {} arguments, expected 1", argcount), node);
                else if (Node& lst = node.list()[1]; lst.nodeType() == NodeType::List)  // only apply len at compile time if we can
                {
                    if (isConstEval(lst))
                    {
                        if (!lst.list().empty() && lst.list()[0] == getListNode())
                            node = Node(static_cast<long>(lst.list().size()) - 1);
                        else
                            node = Node(static_cast<long>(lst.list().size()));
                    }
                }
            }
            else if (name == "@")
            {
                if (node.list().size() != 3)
                    throwMacroProcessingError(fmt::format("Interpreting a `@' with {} arguments, expected 2.", argcount), node);

                Node sublist = evaluate(node.list()[1], is_not_body);
                Node idx = evaluate(node.list()[2], is_not_body);

                if (sublist.nodeType() == NodeType::List && idx.nodeType() == NodeType::Number)
                {
                    long size = static_cast<long>(sublist.list().size());
                    long real_size = size;
                    long num_idx = static_cast<long>(idx.number());

                    // if the first node is the function call to "list", don't count it
                    if (size > 0 && sublist.list()[0] == getListNode())
                    {
                        real_size--;
                        if (num_idx >= 0)
                            ++num_idx;
                    }
                    num_idx = num_idx >= 0 ? num_idx : size + num_idx;

                    if (num_idx < size)
                        return sublist.list()[num_idx];
                    else
                        throwMacroProcessingError(fmt::format("Index ({}) out of range (list size: {})", num_idx, real_size), node);
                }
            }
            else if (name == "head")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `head' inside a macro, got {} arguments, expected 1", argcount), node);
                else if (node.list()[1].nodeType() == NodeType::List)
                {
                    Node& sublist = node.list()[1];
                    if (!sublist.constList().empty() && sublist.constList()[0] == getListNode())
                    {
                        if (sublist.constList().size() > 1)
                        {
                            const Node sublistCopy = sublist.constList()[1];
                            node = sublistCopy;
                        }
                        else
                            node = getNilNode();
                    }
                    else if (!sublist.list().empty())
                        node = sublist.constList()[0];
                    else
                        node = getNilNode();
                }
            }
            else if (name == "tail")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError(fmt::format("When expanding `tail' inside a macro, got {} arguments, expected 1", argcount), node);
                else if (node.list()[1].nodeType() == NodeType::List)
                {
                    Node sublist = node.list()[1];
                    if (!sublist.list().empty() && sublist.list()[0] == getListNode())
                    {
                        if (sublist.list().size() > 1)
                        {
                            sublist.list().erase(sublist.constList().begin() + 1);
                            node = sublist;
                        }
                        else
                        {
                            node = Node(NodeType::List);
                            node.push_back(getListNode());
                        }
                    }
                    else if (!sublist.list().empty())
                    {
                        sublist.list().erase(sublist.constList().begin());
                        node = sublist;
                    }
                    else
                    {
                        node = Node(NodeType::List);
                        node.push_back(getListNode());
                    }
                }
            }
            else if (name == "symcat")
            {
                if (node.list().size() <= 2)
                    throwMacroProcessingError(fmt::format("When expanding `symcat', expected at least 2 arguments, got {} arguments", argcount), node);
                if (node.list()[1].nodeType() != NodeType::Symbol)
                    throwMacroProcessingError(fmt::format("When expanding `symcat', expected the first argument to be a Symbol, got a {}", typeToString(node.list()[1])), node);

                std::string sym = node.list()[1].string();

                for (std::size_t i = 2, end = node.list().size(); i < end; ++i)
                {
                    Node ev = evaluate(node.list()[i], /* is_not_body */ true);

                    switch (ev.nodeType())
                    {
                        case NodeType::Number:
                            sym += std::to_string(static_cast<long int>(ev.number()));  // we don't want '.' in identifiers
                            break;

                        case NodeType::String:
                        case NodeType::Symbol:
                            sym += ev.string();
                            break;

                        default:
                            throwMacroProcessingError(fmt::format("When expanding `symcat', expected either a Number, String or Symbol, got a {}", typeToString(ev)), ev);
                    }
                }

                node.setNodeType(NodeType::Symbol);
                node.setString(sym);
            }
            else if (name == "argcount")
            {
                Node sym = node.constList()[1];
                if (sym.nodeType() == NodeType::Symbol)
                {
                    if (auto it = m_defined_functions.find(sym.string()); it != m_defined_functions.end())
                        node = Node(static_cast<long>(it->second.constList().size()));
                    else
                        throwMacroProcessingError(fmt::format("When expanding `argcount', expected a known function name, got unbound variable {}", sym.string()), sym);
                }
                else if (sym.nodeType() == NodeType::List && sym.list().size() == 3 && sym.list()[0].nodeType() == NodeType::Keyword && sym.list()[0].keyword() == Keyword::Fun)
                    node = Node(static_cast<long>(sym.list()[1].list().size()));
                else
                    throwMacroProcessingError(fmt::format("When trying to apply `argcount', got a {} instead of a Symbol or Function", typeToString(sym)), sym);
            }
            else if (name == "$repr")
            {
                Node ast = node.constList()[1];
                node = Node(NodeType::String, ast.repr());
            }
        }

        if (node.nodeType() == NodeType::List && !node.constList().empty())
        {
            for (auto& child : node.list())
                child = evaluate(child, is_not_body);
        }

        return node;
    }

    bool MacroProcessor::isTruthy(const Node& node)
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            if (node.string() == "true")
                return true;
            else if (node.string() == "false" || node.string() == "nil")
                return false;
        }
        else if ((node.nodeType() == NodeType::Number && node.number() != 0.0) || (node.nodeType() == NodeType::String && !node.string().empty()))
            return true;
        else if (node.nodeType() == NodeType::Spread)
            throwMacroProcessingError("Can not determine the truth value of a spreaded symbol", node);
        return false;
    }

    const Node* MacroProcessor::findNearestMacro(const std::string& name) const
    {
        if (m_macros.empty())
            return nullptr;

        for (auto it = m_macros.rbegin(); it != m_macros.rend(); ++it)
        {
            if (auto res = it->has(name); res != nullptr)
                return res;
        }
        return nullptr;
    }

    void MacroProcessor::deleteNearestMacro(const std::string& name)
    {
        if (m_macros.empty())
            return;

        for (auto it = m_macros.rbegin(); it != m_macros.rend(); ++it)
        {
            if (it->remove(name))
            {
                // stop right here because we found one matching macro
                return;
            }
        }
    }

    bool MacroProcessor::isPredefined(const std::string& symbol)
    {
        auto it = std::find(
            m_predefined_macros.begin(),
            m_predefined_macros.end(),
            symbol);

        return it != m_predefined_macros.end();
    }

    void MacroProcessor::recurApply(Node& node)
    {
        bool applied = applyMacro(node);

        if (applied && node.isListLike())
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
                std::size_t previous = i;

                for (std::size_t block_idx = 1, end = lst.constList().size(); block_idx < end; ++block_idx)
                    node.list().insert(node.constList().begin() + i + block_idx, lst.list()[block_idx]);

                node.list().erase(node.constList().begin() + previous);
            }
        }
    }

    bool MacroProcessor::isConstEval(const Node& node) const
    {
        switch (node.nodeType())
        {
            case NodeType::Symbol:
            {
                auto it = std::find(operators.begin(), operators.end(), node.string());
                auto it2 = std::find_if(Builtins::builtins.begin(), Builtins::builtins.end(),
                                        [&node](const std::pair<std::string, Value>& element) -> bool {
                                            return node.string() == element.first;
                                        });

                return it != operators.end() ||
                    it2 != Builtins::builtins.end() ||
                    findNearestMacro(node.string()) != nullptr ||
                    node.string() == "list";
            }

            case NodeType::List:
                return std::all_of(node.constList().begin(), node.constList().end(), [this](const Node& node) {
                    return isConstEval(node);
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
