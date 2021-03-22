#include <Ark/Compiler/MacroProcessor.hpp>

#include <Ark/Log.hpp>

#include <algorithm>

namespace Ark::internal
{
    MacroProcessor::MacroProcessor(unsigned debug, uint16_t options) noexcept :
        m_debug(debug), m_options(options)
    {
        m_trueNode = Node("true");
        m_falseNode = Node("false");
        m_nilNode = Node("nil");
        m_listNode = Node("list");

        m_trueNode.setNodeType(NodeType::Symbol);
        m_falseNode.setNodeType(NodeType::Symbol);
        m_nilNode.setNodeType(NodeType::Symbol);
        m_listNode.setNodeType(NodeType::Symbol);
    }

    void MacroProcessor::feed(const Node& ast)
    {
        if (m_debug >= 2)
            Ark::logger.info("Processing macros...");

        // to be able to modify it
        m_ast = ast;
        process(m_ast, 0);

        if (m_debug >= 3)
        {
            Ark::logger.info("(MacroProcessor) AST after processing macros");
            std::cout << m_ast << std::endl;
        }
    }

    const Node& MacroProcessor::ast() const noexcept
    {
        return m_ast;
    }

    void MacroProcessor::registerMacro(Node& node)
    {
        // a macro needs at least 2 nodes, name + value is the minimal form
        if (node.const_list().size() < 2)
            throwMacroProcessingError("invalid macro, missing value", node);

        Node& first_node = node.list()[0];
        Node& second_node = node.list()[1];

        // !{name value}
        if (node.const_list().size() == 2)
        {
            if (first_node.nodeType() == NodeType::Symbol)
            {
                m_macros.back()[first_node.string()] = node;
                return;
            }
            throwMacroProcessingError("can not define a macro without a symbol", first_node);
        }
        // !{name (args) body}
        else if (node.const_list().size() == 3 && first_node.nodeType() == NodeType::Symbol)
        {
            if (second_node.nodeType() != NodeType::List)
                throwMacroProcessingError("invalid macro argument's list", second_node);
            else
            {
                bool had_spread = false;
                for (const Node& n : second_node.const_list())
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
        else if (std::size_t sz = node.const_list().size(); sz == 3 || sz == 4)
        {
            if (first_node.nodeType() == NodeType::Keyword && first_node.keyword() == Keyword::If)
            {
                execute(node);
                return;
            }
            else if (first_node.nodeType() == NodeType::Keyword)
                throwMacroProcessingError("the only authorized keyword in macros is `if'", first_node);
        }
        // if we are here, it means we couldn't recognize the given macro, thus making it invalid
        throwMacroProcessingError("unrecognized macro form", node);
    }

    void MacroProcessor::process(Node& node, unsigned depth)
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
                    if ((!m_macros.empty() && !m_macros.back().empty()) || !has_created)
                    {
                        has_created = true;
                        m_macros.emplace_back();
                        m_macros.back()["#depth"] = Node(static_cast<double>(depth));
                    }

                    registerMacro(node.list()[i]);
                    if (node.list()[i].nodeType() == NodeType::Macro)
                        node.list().erase(node.const_list().begin() + i);
                }
                else
                {
                    // execute only if we have registered macros
                    if ((m_macros.size() == 1 && m_macros[0].size() > 0) || m_macros.size() > 1)
                        execute(node.list()[i]);

                    process(node.list()[i], depth + 1);

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

    void MacroProcessor::execute(Node& node)
    {
        static std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> apply_to =
        [](const std::unordered_map<std::string, Node>& map, Node& target, Node* parent) {
            if (target.nodeType() == NodeType::Symbol)
            {
                if (auto p = map.find(target.string()); p != map.end())
                    target = p->second;
            }
            else if (target.nodeType() == NodeType::List)
            {
                for (std::size_t i = 0, end = target.list().size(); i < end; ++i)
                    apply_to(map, target.list()[i], &target);
            }
            else if (target.nodeType() == NodeType::Spread)
            {
                Node subnode = target;
                subnode.setNodeType(NodeType::Symbol);
                apply_to(map, subnode, parent);
                parent->list().pop_back();  // remove the spread
                for (std::size_t i = 1, end = subnode.list().size(); i < end; ++i)
                    parent->push_back(subnode.list()[i]);
            }
        };

        if (node.nodeType() == NodeType::Symbol)
        {
            // error ?
            Node* macro = find_nearest_macro(node.string());

            if (macro != nullptr)
            {
                if (m_debug >= 3)
                    Ark::logger.info("Found macro for", node.string());

                // !{name value}
                if (macro->const_list().size() == 2)
                    node = macro->list()[1];
            }

            return;
        }
        else if (node.nodeType() == NodeType::Macro && node.list()[0].nodeType() == NodeType::Keyword)
        {
            Node& first = node.list()[0];

            if (first.keyword() == Keyword::If)
            {
                Node& cond = node.list()[1];
                Node temp = evaluate(cond, /* is_not_body */ true);

                // evaluate cond
                if (isTruthy(temp))
                    node = node.list()[2];
                else if (node.const_list().size() > 2)
                    node = node.list()[3];
                else
                {
                    // remove node because nothing matched
                    node.list().clear();
                    node.setNodeType(NodeType::List);
                }

                if (node.nodeType() == NodeType::Macro)
                    registerMacro(node);
            }
        }
        else if (node.nodeType() == NodeType::List && node.const_list().size() > 0
                && node.list()[0].nodeType() == NodeType::Symbol)
        {
            Node& first = node.list()[0];
            Node* macro = find_nearest_macro(first.string());

            if (macro != nullptr)
            {
                if (m_debug >= 3)
                    Ark::logger.info("Found macro for", first.string());

                if (macro->const_list().size() == 2)
                    execute(first);
                // !{name (args) body}
                else if (macro->const_list().size() == 3)
                {
                    Node temp_body = macro->const_list()[2];
                    Node args = macro->const_list()[1];

                    // bind node->list() to temp_body using macro->const_list()[1]
                    std::unordered_map<std::string, Node> args_applied;
                    std::size_t j = 0;
                    for (std::size_t i = 1, end = node.const_list().size(); i < end; ++i)
                    {
                        const std::string& arg_name = args.list()[j].string();
                        if (args.list()[j].nodeType() == NodeType::Symbol)
                        {
                            args_applied[arg_name] = node.const_list()[i];
                            ++j;
                        }
                        else if (args.list()[j].nodeType() == NodeType::Spread)
                        {
                            if (args_applied.find(arg_name) == args_applied.end())
                            {
                                args_applied[arg_name] = Node(NodeType::List);
                                args_applied[arg_name].push_back(m_listNode);
                            }
                            // do not move j because we checked before that the spread is always the last one
                            args_applied[arg_name].push_back(node.const_list()[i]);
                        }
                    }

                    // check argument count
                    if (args_applied.size() + 1 == args.list().size() && args.list().back().nodeType() == NodeType::Spread)
                    {
                        // just a spread we didn't assign
                        args_applied[args.list().back().string()] = Node(NodeType::List);
                        args_applied[args.list().back().string()].push_back(m_listNode);
                    }
                    else if (args_applied.size() != args.list().size())
                        throwMacroProcessingError("Macro `" + macro->const_list()[0].string() + "' got " + std::to_string(args_applied.size()) + " argument(s) but needed " + std::to_string(args.list().size()), *macro);

                    if (!args_applied.empty())
                        apply_to(args_applied, temp_body, nullptr);
                    node = evaluate(temp_body);
                    execute(node);
                }
            }
        }
    }

    Node MacroProcessor::evaluate(Node& node, bool is_not_body)
    {
        if (node.nodeType() == NodeType::Symbol)
        {
            Node* macro = find_nearest_macro(node.string());
            if (macro != nullptr && macro->list().size() == 2)
                return macro->list()[1];
            else
                return node;
        }
        else if (node.nodeType() == NodeType::List && node.const_list().size() > 1
            && node.list()[0].nodeType() == NodeType::Symbol
            && is_not_body)
        {
            const std::string& name = node.list()[0].string();
            if (Node* macro = find_nearest_macro(name); macro != nullptr)
            {
                execute(node.list()[0]);
            }
            else if (name == "=")
            {
                if (node.list().size() != 3)
                    throwMacroProcessingError("Interpreting a `=' condition with " + std::to_string(node.list().size() - 1) + " arguments, instead of 2.", node);

                return (evaluate(node.list()[1], is_not_body) == evaluate(node.list()[2], is_not_body)) ? m_trueNode : m_falseNode;
            }
            else if (name == "!=")
            {
                if (node.list().size() != 3)
                    throwMacroProcessingError("Interpreting a `!=' condition with " + std::to_string(node.list().size() - 1) + " arguments, instead of 2.", node);

                return (evaluate(node.list()[1], is_not_body) != evaluate(node.list()[2], is_not_body)) ? m_trueNode : m_falseNode;
            }
            else if (name == "<")
            {}
            else if (name == ">")
            {}
            else if (name == "<=")
            {}
            else if (name == ">=")
            {}
            else if (name == "not")
            {
                if (node.list().size() != 2)
                    throwMacroProcessingError("Interpreting a `not' condition with " + std::to_string(node.list().size() - 1) + " arguments, instead of 1.", node);

                return (!isTruthy(evaluate(node.list()[1], is_not_body))) ? m_trueNode : m_falseNode;
            }
            else if (name == "and")
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError("Interpreting a `and' chain with " + std::to_string(node.list().size() - 1) + " arguments, expected at least 2.", node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (!isTruthy(evaluate(node.list()[i], is_not_body)))
                        return m_falseNode;
                }
                return m_trueNode;
            }
            else if (name == "or")
            {
                if (node.list().size() < 3)
                    throwMacroProcessingError("Interpreting a `or' chain with " + std::to_string(node.list().size() - 1) + " arguments, expected at least 2.", node);

                for (std::size_t i = 1, end = node.list().size(); i < end; ++i)
                {
                    if (isTruthy(evaluate(node.list()[i], is_not_body)))
                        return m_trueNode;
                }
                return m_falseNode;
            }
            else if (name == "len")
            {
                if (node.list().size() > 2)
                    throwMacroProcessingError("When expanding `len' inside a macro, got " + std::to_string(node.list().size() - 1) + " arguments, needed only 1", node);
                else if (node.list()[1].nodeType() != NodeType::List)
                    throwMacroProcessingError("When expanding `len' inside a macro, got a " + typeToString(node.list()[1]) + ", needed a List", node);

                Node& sublist = node.list()[1];
                if (sublist.list().size() > 0 && sublist.list()[0] == m_listNode)
                    node = Node(static_cast<int>(sublist.list().size()) - 1);
                else
                    node = Node(static_cast<int>(sublist.list().size()));
            }
            else if (name == "@")
            {
                if (node.list().size() != 3)
                    throwMacroProcessingError("Interpreting a `@' with " + std::to_string(node.list().size() - 1) + " arguments, instead of 2.", node);

                Node sublist = evaluate(node.list()[1], is_not_body);
                Node idx = evaluate(node.list()[2], is_not_body);

                if (sublist.nodeType() != NodeType::List)
                    throwMacroProcessingError("Interpreting a `@' with a " + typeToString(sublist) + " instead of a List", sublist);
                if (idx.nodeType() != NodeType::Number)
                    throwMacroProcessingError("Interpreting a `@' with a " + typeToString(idx) + " as the index type, instead of a Number", idx);

                long num_idx = static_cast<long>(idx.number());
                long sz = static_cast<long>(sublist.list().size());
                long offset = 0;
                if (sz > 0 && sublist.list()[0] == m_listNode)
                {
                    num_idx = (num_idx >= 0) ? num_idx + 1 : num_idx;
                    offset = -1;
                }

                if (num_idx < 0 && sz + num_idx + offset >= 0)
                    return sublist.list()[sz + num_idx - offset];
                else if (num_idx >= 0 && num_idx < sz + offset)
                    return sublist.list()[num_idx - offset];

                throwMacroProcessingError("Index error when processing `@' in macro: got index " + std::to_string(num_idx) + ", while max size was " + std::to_string(sz + offset), node);
            }
            else if (name == "head")
            {}
            else if (name == "tail")
            {}
            else
            {
                // TODO work on the other elements of the list
            }
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