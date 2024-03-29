#include <Ark/Compiler/Macros/Executors/Conditional.hpp>

namespace Ark::internal
{
    bool ConditionalExecutor::applyMacro(Node& node)
    {
        Node cond = node.list()[1];
        Node temp = evaluate(cond, /* is_not_body */ true);
        Node if_true = node.list()[2];
        Node if_false = node.constList().size() > 3 ? node.list()[3] : getNilNode();

        // evaluate cond
        if (isTruthy(temp))
            node = if_true;
        else if (node.constList().size() > 3)
            node = if_false;
        else
        {
            // remove node because nothing matched
            node.list().clear();
            node.setNodeType(NodeType::Unused);
        }

        // todo: is this still needed? does registerMacro have to call applyMacro???
        if (node.nodeType() == NodeType::Macro)
            registerMacro(node);

        return true;
    }

    bool ConditionalExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Macro && node.list()[0].nodeType() == NodeType::Keyword && node.list()[0].keyword() == Keyword::If;
    }
}
