#include <Ark/Compiler/MacroExecutors/ListExecutor.hpp>
#include <Ark/Log.hpp>

namespace Ark::internal 
{
    void ListExecutor::execute(
        std::function<Node*(const std::string& name)> const& find_nearest_macro, 
        std::function<void(Node &node)> const& registerMacro,
        std::function<bool(const Node& node)> const& isTruthy,
        std::function<Node(Node& node, bool is_not_body)> const& evaluate,
        std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> const& apply_to,
        std::function<void(const std::string& message, const Node& node)> const& throwMacroProcessingError,
        std::function<void(Node& node)> const& func_execute,
        Node &node) 
        {
            if (node.nodeType() == NodeType::List && node.const_list().size() > 0){
            Node& first = node.list()[0];
            Node* macro = find_nearest_macro(first.string());

            if (macro != nullptr)
            {
                if (m_debug >= 3)
                    Ark::logger.info("Found macro for", first.string());

                if (macro->const_list().size() == 2)
                    func_execute(first);
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
                                args_applied[arg_name].push_back(Node::ListNode);
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
                        args_applied[args.list().back().string()].push_back(Node::ListNode);
                    }
                    else if (args_applied.size() != args.list().size())
                    {
                        std::size_t args_needed = args.list().size();
                        std::string macro_name = macro->const_list()[0].string();

                        if (args.list().back().nodeType() != NodeType::Spread)
                            throwMacroProcessingError("Macro `" + macro_name + "' got " + std::to_string(args_applied.size()) + " argument(s) but needed " + std::to_string(args_needed), *macro);
                        else
                            throwMacroProcessingError("Macro `" + macro_name + "' got " + std::to_string(args_applied.size()) + " argument(s) but needed at least " + std::to_string(args_needed - 1), *macro);
                    }

                    if (!args_applied.empty())
                        apply_to(args_applied, temp_body, nullptr);
                    node = evaluate(temp_body, false);
                    func_execute(node);
                }
            }
            }
    }
}
