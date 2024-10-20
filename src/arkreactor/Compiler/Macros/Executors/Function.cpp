#include <Ark/Compiler/Macros/Executors/Function.hpp>

#include <fmt/core.h>
#include <ranges>

namespace Ark::internal
{
    bool FunctionExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Symbol;
    }

    bool FunctionExecutor::applyMacro(Node& node, const unsigned depth)
    {
        Node& first = node.list()[0];

        if (const Node* macro = findNearestMacro(first.string()); macro != nullptr)
        {
            if (macro->constList().size() == 2)
                applyMacroProxy(first, depth + 1);
            // ($ name (args) body)
            else if (macro->constList().size() == 3)
            {
                Node temp_body = macro->constList()[2];
                Node args = macro->constList()[1];
                std::size_t args_needed = args.list().size();
                std::size_t args_given = node.constList().size() - 1;  // remove the first (the name of the macro)
                std::string macro_name = macro->constList()[0].string();
                const bool has_spread = args_needed > 0 && args.list().back().nodeType() == NodeType::Spread;

                // bind node->list() to temp_body using macro->constList()[1]
                std::unordered_map<std::string, Node> args_applied;
                std::size_t j = 0;
                for (std::size_t i = 1, end = node.constList().size(); i < end; ++i)
                {
                    // by breaking early if we have too many arguments, the args_applied/args_needed check will fail
                    if (j >= args_needed)
                        break;

                    const std::string& arg_name = args.list()[j].string();
                    if (args.list()[j].nodeType() == NodeType::Symbol)
                    {
                        args_applied[arg_name] = node.constList()[i];
                        ++j;
                    }
                    else if (args.list()[j].nodeType() == NodeType::Spread)
                    {
                        if (!args_applied.contains(arg_name))
                        {
                            args_applied[arg_name] = Node(NodeType::List);
                            args_applied[arg_name].push_back(getListNode());
                        }
                        // do not move j because we checked before that the spread is always the last one
                        args_applied[arg_name].push_back(node.constList()[i]);
                    }
                }

                // check argument count
                if (args_applied.size() + 1 == args_needed && has_spread)
                {
                    // just a spread we didn't assign
                    args_applied[args.list().back().string()] = Node(NodeType::List);
                    args_applied[args.list().back().string()].push_back(getListNode());
                }

                if (args_given != args_needed && !has_spread)
                    throwMacroProcessingError(fmt::format("Macro `{}' got {} argument(s) but needed {}", macro_name, args_given, args_needed), node);
                if (args_applied.size() != args_needed && has_spread)
                    // args_needed - 1 because we do not count the spread as a required argument
                    throwMacroProcessingError(fmt::format("Macro `{}' got {} argument(s) but needed at least {}", macro_name, args_applied.size(), args_needed - 1), node);

                if (!args_applied.empty())
                    unify(args_applied, temp_body, nullptr);

                setWithFileAttributes(node, node, evaluate(temp_body, depth + 1, false));
                applyMacroProxy(node, depth + 1);  // todo: this seems useless
                return true;
            }
        }
        else if (std::ranges::find(Language::macros, first.string()) != Language::macros.end())
        {
            setWithFileAttributes(node, node, evaluate(node, depth + 1, false));
            return true;
        }

        return false;
    }
}
