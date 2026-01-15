#include <cassert>
#include <Ark/Compiler/Macros/Executors/Function.hpp>

#include <fmt/core.h>
#include <ranges>

#include <Ark/Constants.hpp>

namespace Ark::internal
{
    bool FunctionExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::List && !node.constList().empty() && node.constList()[0].nodeType() == NodeType::Symbol;
    }

    bool FunctionExecutor::applyMacro(Node& node, const unsigned depth)
    {
        const Node& first = node.list()[0];

        // (macro name (args) body)
        if (const Node* macro = findNearestMacro(first.string()); macro != nullptr && macro->constList().size() == 3)
        {
            Node temp_body = macro->constList()[2];
            Node args = macro->constList()[1];
            const std::size_t args_needed = args.list().size();
            const std::size_t args_given = node.constList().size() - 1;  // remove the first (the name of the macro)
            const std::string macro_name = macro->constList()[0].string();
            // thanks to the parser, we are guaranteed that the spread will be in last position, if any
            const bool has_spread = args_needed > 0 && args.list().back().nodeType() == NodeType::Spread;

            // save the args given to the macro by giving them a name (from the macro args block),
            // and a value (in node.constList())
            std::unordered_map<std::string, Node> args_applied;
            std::size_t j = 0;
            for (std::size_t i = 1, end = node.constList().size(); i < end; ++i)
            {
                // by breaking early if we have too many arguments, the args_applied/args_needed check will fail
                if (j >= args_needed)
                    break;

                if (args.list()[j].nodeType() == NodeType::Symbol)
                {
                    const std::string& arg_name = args.list()[j].string();
                    args_applied[arg_name] = node.constList()[i];
                    ++j;
                }
                else if (args.list()[j].nodeType() == NodeType::Spread)
                {
                    const std::string& arg_name = args.list()[j].string();
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

            node.updateValueAndType(evaluate(temp_body, depth + 1, false));
            applyMacroProxy(node, depth + 1);
            return true;
        }

        if (std::ranges::find(Language::macros, first.string()) != Language::macros.end())
        {
            node.updateValueAndType(evaluate(node, depth + 1, false));
            return true;
        }

        return false;
    }

    Node FunctionExecutor::macroNode(Node& node)
    {
        const Node& first = node.list()[0];
        if (const Node* macro = findNearestMacro(first.string()); macro != nullptr && macro->constList().size() == 3)
            return *macro;
        return {};
    }

    void FunctionExecutor::unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, const std::size_t index, const std::size_t unify_depth)
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
            if (target.nodeType() == NodeType::Macro && target.list()[0].nodeType() == NodeType::Symbol)
            {
                if (const std::string macro_name = target.list()[0].string(); map.contains(macro_name))
                    throwMacroProcessingError(
                        fmt::format(
                            "Can not define a macro by reusing the argument name `{}'",
                            macro_name),
                        target);

                // proceed for expansion only on the value of each macro
                unify(map, target.list().back(), &target, target.list().size() - 1, unify_depth + 1);
            }
            else
            {
                // proceed for expansion on normal nodes, we can safely run on all subnodes
                for (std::size_t i = 0; i < target.list().size(); ++i)
                    unify(map, target.list()[i], &target, i, unify_depth + 1);
            }
        }
        else if (target.nodeType() == NodeType::Spread)
        {
            assert(parent != nullptr && "Parent node should be defined when unifying a spread");

            Node sub_node = target;
            sub_node.setNodeType(NodeType::Symbol);
            unify(map, sub_node, parent, 0, unify_depth + 1);

            if (sub_node.nodeType() != NodeType::List)
                parent->list()[index] = sub_node;
            else
            {
                const bool is_list = sub_node.list().front() == getListNode();

                for (std::size_t i = is_list ? 1 : 0, end = sub_node.list().size(); i < end; ++i)
                    parent->list().insert(
                        parent->list().begin() + static_cast<std::vector<Node>::difference_type>(index + i + (is_list ? 0 : 1)),
                        sub_node.list()[i]);
                // remove the spread
                parent->list().erase(parent->list().begin() + static_cast<std::vector<Node>::difference_type>(index));
            }
        }
    }
}
