#include <Ark/Compiler/MacroProcessor.hpp>
#include <Ark/Compiler/MacroExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/SymbolExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/ListExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {
        // initialize default Nodes
        Node::init();

        // create executors pipeline
        std::vector<std::shared_ptr<MacroExecutor>> executors = {
            std::make_shared<SymbolExecutor>(this),
            std::make_shared<ConditionalExecutor>(this),
            std::make_shared<ListExecutor>(this)
        };
        m_executor_pipeline = std::make_unique<MacroExecutorPipeline>(executors);

        m_predefined_macros = {
            "symcat",
            "argcount"
        };
    }

    void MacroProcessor::feed(const Node& ast)
    {
        if (m_debug >= 2)
            std::cout << "Processing macros...\n";

        // to be able to modify it
        m_ast = ast;
        process(m_ast, 0);

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
            throwMacroProcessingError("invalid macro, missing value", node);

        Node& first_node = node.list()[0];
        Node& second_node = node.list()[1];

        // !{name value}
        if (node.constList().size() == 2)
        {
            if (first_node.nodeType() == NodeType::Symbol)
            {
                if (first_node.string() != "undef")
                    m_macros.back()[first_node.string()] = node;
                else if (second_node.nodeType() == NodeType::Symbol)  // undefine a macro
                    deleteNearestMacro(second_node.string());
                else // used undef on a non-symbol
                    throwMacroProcessingError("can not undefine a macro without it's name", second_node);
                return;
            }
            throwMacroProcessingError("can not define a macro without a symbol", first_node);
        }
        // !{name (args) body}
        else if (node.constList().size() == 3 && first_node.nodeType() == NodeType::Symbol)
        {
            if (second_node.nodeType() != NodeType::List)
                throwMacroProcessingError("invalid macro argument's list", second_node);
            else
            {
                bool had_spread = false;
                for (const Node& n : second_node.constList())
                {
                    if (n.nodeType() != NodeType::Symbol && n.nodeType() != NodeType::Spread)
                        throwMacroProcessingError("invalid macro argument's list, expected symbols", n);
                    else if (n.nodeType() == NodeType::Spread)
                    {
                        if (had_spread)
                            throwMacroProcessingError("got another spread argument, only one is allowed", n);
                        had_spread = true;
                    }
                    else if (had_spread && n.nodeType() == NodeType::Symbol)
                        throwMacroProcessingError("got another argument after a spread argument, which is invalid", n);
                }
                m_macros.back()[first_node.string()] = node;
                return;
            }
        }
        // !{if cond then else}
        else if (std::size_t sz = node.constList().size(); sz == 3 || sz == 4)
        {
            if (first_node.nodeType() == NodeType::Keyword && first_node.keyword() == Keyword::If)
            {
                applyMacro(node);
                return;
            }
            else if (first_node.nodeType() == NodeType::Keyword)
                throwMacroProcessingError("the only authorized keyword in macros is `if'", first_node);
        }
        // if we are here, it means we couldn't recognize the given macro, thus making it invalid
        throwMacroProcessingError("unrecognized macro form", node);
    }

    void MacroProcessor::registerFuncDef(Node& node)
    {
        if (node.nodeType() == NodeType::List && node.constList().size() > 0 && node.constList()[0].nodeType() == NodeType::Keyword)
        {
            Keyword kw = node.constList()[0].keyword();
            if (kw != Keyword::Let && kw != Keyword::Mut && kw != Keyword::Set)
                return;

            const Node& inner = node.constList()[2];
            if (inner.nodeType() != NodeType::List)
                return;

            if (inner.constList()[0].nodeType() == NodeType::Keyword && inner.constList()[0].keyword() == Keyword::Fun)
                m_defined_functions[node.constList()[1].string()] = inner.constList()[1];
        }
    }

    void MacroProcessor::process(Node& node, unsigned depth)
    {
        bool has_created = false;

        if (node.nodeType() == NodeType::List)
        {
            // register known functions
            registerFuncDef(node);

            // recursive call
            std::size_t i = 0;
            while (i < node.list().size())
            {
                if (node.list()[i].nodeType() == NodeType::Macro)
                {
                    // create a scope only if needed
                    if ((!m_macros.empty() && !m_macros.back().empty()) || !has_created)
                    {
                        has_created = true;
                        m_macros.emplace_back();
                        m_macros.back()["#depth"] = Node(static_cast<double>(depth));
                    }

                    registerMacro(node.list()[i]);
                    if (node.list()[i].nodeType() == NodeType::Macro || node.list()[i].nodeType() == NodeType::Unused)
                        node.list().erase(node.constList().begin() + i);
                }
                else  // running on non-macros
                {
                    bool added_begin = false;

                    // apply macro only if we have registered macros
                    if ((m_macros.size() == 1 && m_macros[0].size() > 0) || m_macros.size() > 1)
                    {
                        bool had = hadBegin(node.list()[i]);
                        bool applied = applyMacro(node.list()[i]);

                        // remove unused blocks
                        if (node.list()[i].nodeType() == NodeType::Unused)
                            node.list().erase(node.constList().begin() + i);
                        // if we got `macro`, it was replaced but not entirely applied.
                        // but `(macro)` would get entirely applied because it's in a list,
                        // thus we need to evaluate the node if we have list[i].list[0] as a macro
                        if (applied)
                            recurApply(node.list()[i]);

                        if (hadBegin(node.list()[i]) && !had)
                            added_begin = true;
                    }

                    if (node.nodeType() == NodeType::List)
                    {
                        process(node.list()[i], depth + 1);
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
            if (!m_macros.empty() && !m_macros.back().empty() && static_cast<unsigned>(m_macros.back()["#depth"].number()) == depth)
                m_macros.pop_back();
        }
    }

    bool MacroProcessor::applyMacro(Node& node)
    {
        return m_executor_pipeline->applyMacro(node);
    }

    void MacroProcessor::unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, std::size_t index)
    {
        if (target.nodeType() == NodeType::Symbol)
        {
            if (auto p = map.find(target.string()); p != map.end())
                target = p->second;
        }
        else if (target.nodeType() == NodeType::List || target.nodeType() == NodeType::Macro)
        {
            for (std::size_t i = 0, end = target.list().size(); i < end; ++i)
                unify(map, target.list()[i], &target, i);
        }
        else if (target.nodeType() == NodeType::Spread)
        {
            Node subnode = target;
            subnode.setNodeType(NodeType::Symbol);
            unify(map, subnode, parent);

            if (subnode.nodeType() != NodeType::List)
                throwMacroProcessingError("Got a non-list while trying to apply the spread operator", subnode);

            for (std::size_t i = 1, end = subnode.list().size(); i < end; ++i)
                parent->list().insert(parent->list().begin() + index + i, subnode.list()[i]);
            parent->list().erase(parent->list().begin() + index);  // remove the spread
        }
    }

    Node MacroProcessor::evaluate(Node& node, bool is_not_body)
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            Node* macro = findNearestMacro(node.string());
            if (macro != nullptr && macro->list().size() == 2)
                return macro->list()[1];
            else
                return node;
        }
        else if (node.nodeType() == NodeType::List && node.constList().size() > 1 && node.list()[0].nodeType() == NodeType::Symbol)
        {
            #define GEN_NOT_BODY(str_name, error_handler, ret)                \
                else if (name == str_name && is_not_body) {                   \
                    if (node.list().size() != 3) error_handler;               \
                    Node one = evaluate(node.list()[1], is_not_body),         \
                         two = evaluate(node.list()[2], is_not_body);         \
                    return ret;                                               \
                }

            #define GEN_COMPARATOR(str_name, cond) GEN_NOT_BODY(                                    \
                str_name,                                                                           \
                throwMacroProcessingError("Interpreting a `" str_name "' condition with " +         \
                    std::to_string(node.list().size() - 1) + " arguments, instead of 2.", node),    \
                (cond) ? Node::TrueNode : Node::FalseNode                                           \
            )

            #define GEN_OP(str_name, op) GEN_NOT_BODY(                                              \
                str_name,                                                                           \
                throwMacroProcessingError("Interpreting a `" str_name "' operation with " +         \
                    std::to_string(node.list().size() - 1) + " arguments, instead of 2.", node),    \
                (one.nodeType() == two.nodeType() && two.nodeType() == NodeType::Number) ? Node(one.number() op two.number()) : node \
            )

            const std::string& name = node.list()[0].string();
            if (Node* macro = findNearestMacro(name); macro != nullptr)
            {
                applyMacro(node.list()[0]);
                if (node.list()[0].nodeType() == NodeType::Unused)
                    node.list().erase(node.constList().begin());
            }
            GEN_COMPARATOR("=",    one == two)
            GEN_COMPARATOR("!=", !(one == two))
            GEN_COMPARATOR("<",    one <  two)
            GEN_COMPARATOR(">",  !(one <  two) && !(one == two))
            GEN_COMPARATOR("<=",   one <  two ||    one == two)
            GEN_COMPARATOR(">=", !(one <  two))
            GEN_OP("+", +)
            GEN_OP("-", -)
            GEN_OP("*", *)
            GEN_OP("/", /)
            else if (name == "not" && is_not_body)
            {
                if (node.list().size() != 2)
                    throwMacroProcessingError("Interpreting a `not' condition with " + std::to_string(node.list().size() - 1) + " arguments, instead of 1.", node);

                return (!isTruthy(evaluate(node.list()[1], is_not_body))) ? Node::TrueNode : Node::FalseNode;
            }
            else if (name == "and" && is_not_body)
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError("Interpreting a `and' chain with " + std::to_string(node.list().size() - 1) + " arguments, expected at least 2.", node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (!isTruthy(evaluate(node.list()[i], is_not_body)))
                        return Node::FalseNode;
                }
                return Node::TrueNode;
            }
            else if (name == "or" && is_not_body)
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError("Interpreting a `or' chain with " + std::to_string(node.list().size() - 1) + " arguments, expected at least 2.", node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (isTruthy(evaluate(node.list()[i], is_not_body)))
                        return Node::TrueNode;
                }
                return Node::FalseNode;
            }
            else if (name == "len")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError("When expanding `len' inside a macro, got " + std::to_string(node.list().size() - 1) + " arguments, needed only 1", node);
                else if (Node& lst = node.list()[1]; lst.nodeType() == NodeType::List)  // only apply len at compile time if we can
                {
                    if (canBeCompileTimeEvaluated(lst))
                    {
                        if (lst.list().size() > 0 && lst.list()[0] == Node::ListNode)
                            node = Node(static_cast<long>(lst.list().size()) - 1);
                        else
                            node = Node(static_cast<long>(lst.list().size()));
                    }
                }
            }
            else if (name == "@")
            {
                if (node.list().size() != 3)
                    throwMacroProcessingError("Interpreting a `@' with " + std::to_string(node.list().size() - 1) + " arguments, instead of 2.", node);

                Node sublist = evaluate(node.list()[1], is_not_body);
                Node idx = evaluate(node.list()[2], is_not_body);

                if (sublist.nodeType() == NodeType::List && idx.nodeType() == NodeType::Number)
                {
                    long num_idx = static_cast<long>(idx.number());
                    long sz = static_cast<long>(sublist.list().size());
                    long offset = 0;
                    if (sz > 0 && sublist.list()[0] == Node::ListNode)
                    {
                        num_idx = (num_idx >= 0) ? num_idx + 1 : num_idx;
                        offset = -1;
                    }

                    if (num_idx < 0 && sz + num_idx >= 0 && -num_idx < sz)
                        return sublist.list()[sz + num_idx];
                    else if (num_idx >= 0 && num_idx + offset < sz)
                        return sublist.list()[num_idx];
                }
            }
            else if (name == "head")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError("When expanding `head' inside a macro, got " + std::to_string(node.list().size() - 1) + " arguments, needed only 1", node);
                else if (node.list()[1].nodeType() == NodeType::List)
                {
                    Node& sublist = node.list()[1];
                    if (sublist.constList().size() > 0 && sublist.constList()[0] == Node::ListNode)
                    {
                        if (sublist.constList().size() > 1)
                        {
                            const Node sublistCopy = sublist.constList()[1];
                            node = sublistCopy;
                        }
                        else
                            node = Node::NilNode;
                    }
                    else if (sublist.list().size() > 0)
                        node = sublist.constList()[0];
                    else
                        node = Node::NilNode;
                }
            }
            else if (name == "tail")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError("When expanding `tail' inside a macro, got " + std::to_string(node.list().size() - 1) + " arguments, needed only 1", node);
                else if (node.list()[1].nodeType() == NodeType::List)
                {
                    Node sublist = node.list()[1];
                    if (sublist.list().size() > 0 && sublist.list()[0] == Node::ListNode)
                    {
                        if (sublist.list().size() > 1)
                        {
                            sublist.list().erase(sublist.constList().begin() + 1);
                            node = sublist;
                        }
                        else
                        {
                            node = Node(NodeType::List);
                            node.push_back(Node::ListNode);
                        }
                    }
                    else if (sublist.list().size() > 0)
                    {
                        sublist.list().erase(sublist.constList().begin());
                        node = sublist;
                    }
                    else
                    {
                        node = Node(NodeType::List);
                        node.push_back(Node::ListNode);
                    }
                }
            }
            else if (name == "symcat")
            {
                if (node.list().size() <= 2)
                    throwMacroProcessingError("When expanding `symcat', expected at least 2 arguments, got " + std::to_string(node.list().size() - 1) + " arguments", node);
                if (node.list()[1].nodeType() != NodeType::Symbol)
                    throwMacroProcessingError("When expanding `symcat', expected the first argument to be a Symbol, got a " + typeToString(node.list()[1]), node);

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
                            throwMacroProcessingError("When expanding `symcat', expected either a Number, String or Symbol, got a " + typeToString(ev), ev);
                    }
                }

                node.setNodeType(NodeType::Symbol);
                node.setString(sym);
            }
            else if (name == "argcount")
            {
                Node sym = node.constList()[1];
                if (auto it = m_defined_functions.find(sym.string()); it != m_defined_functions.end())
                    node = Node(static_cast<long>(it->second.constList().size()));
                else
                    throwMacroProcessingError("When expanding `argcount', expected a known function name, got unbound variable " + sym.string(), node);
            }
        }

        if (node.nodeType() == NodeType::List && node.constList().size() >= 1)
        {
            for (std::size_t i = 0; i < node.list().size(); ++i)
                node.list()[i] = evaluate(node.list()[i], is_not_body);
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
        else if (node.nodeType() == NodeType::Number && node.number() != 0.0)
            return true;
        else if (node.nodeType() == NodeType::String && node.string().size() != 0)
            return true;
        else if (node.nodeType() == NodeType::Spread)
            throwMacroProcessingError("Can not determine the truth value of a spreaded symbol", node);
        return false;
    }
}