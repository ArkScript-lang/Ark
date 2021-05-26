#include <Ark/Compiler/MacroExecutors/ListExecutor.hpp>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    ListExecutor::ListExecutor(MacroProcessor* macroprocessor, unsigned debug) :
        MacroExecutor(macroprocessor, debug)
    {}

    bool ListExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::List && node.constList().size() > 0 && node.constList()[0].nodeType() == NodeType::Symbol;
    }

    bool ListExecutor::applyMacro(Node& node)
    {
        Node& first = node.list()[0];
        Node* macro = findNearestMacro(first.string());

        if (macro != nullptr)
        {
            if (m_debug >= 3)
                std::clog << "Found macro for " << first.string() << std::endl;

            if (macro->constList().size() == 2)
                applyMacroProxy(first);
            // !{name (args) body}
            else if (macro->constList().size() == 3)
            {
                Node temp_body = macro->constList()[2];
                Node args = macro->constList()[1];

                // bind node->list() to temp_body using macro->constList()[1]
                std::unordered_map<std::string, Node> args_applied;
                std::size_t j = 0;
                for (std::size_t i = 1, end = node.constList().size(); i < end; ++i)
                {
                    const std::string& arg_name = args.list()[j].string();
                    if (args.list()[j].nodeType() == NodeType::Symbol)
                    {
                        args_applied[arg_name] = node.constList()[i];
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
                        args_applied[arg_name].push_back(node.constList()[i]);
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
                    std::string macro_name = macro->constList()[0].string();

                    if (args.list().back().nodeType() != NodeType::Spread)
                        throwMacroProcessingError("Macro `" + macro_name + "' got " + std::to_string(args_applied.size()) + " argument(s) but needed " + std::to_string(args_needed), *macro);
                    else
                        throwMacroProcessingError("Macro `" + macro_name + "' got " + std::to_string(args_applied.size()) + " argument(s) but needed at least " + std::to_string(args_needed - 1), *macro);
                }

                if (!args_applied.empty())
                    unify(args_applied, temp_body, nullptr);

                node = evaluate(temp_body, false);
                applyMacroProxy(node);
                return true;
            }
        }
        else if (isPredefined(first.string()))
        {
            node = evaluate(node, false);
            return true;
        }

        return false;
    }
}
