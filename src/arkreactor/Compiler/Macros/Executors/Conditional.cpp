#include <Ark/Compiler/Macros/Executors/Conditional.hpp>

namespace Ark::internal
{
    bool ConditionalExecutor::applyMacro(Node& node, unsigned depth)
    {
        Node cond = node.list()[1];
        const Node temp = evaluate(cond, depth + 1, /* is_not_body */ true);
        const Node if_true = node.list()[2];
        const Node if_false = node.constList().size() > 3 ? node.list()[3] : getNilNode();

        // evaluate cond
        if (isTruthy(temp))
            setWithFileAttributes(node, node, if_true);
        else if (node.constList().size() > 3)
            setWithFileAttributes(node, node, if_false);
        else
        {
            // remove node because nothing matched
            node.list().clear();
            node.setNodeType(NodeType::Unused);
        }

        if (node.nodeType() == NodeType::Macro)
            registerMacro(node);

        return true;
    }

    bool ConditionalExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Macro && node.list()[0].nodeType() == NodeType::Keyword && node.list()[0].keyword() == Keyword::If;
    }
}
