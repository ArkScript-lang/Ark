#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>

namespace Ark::internal
{
    ConditionalExecutor::ConditionalExecutor(MacroProcessor* macroprocessor, unsigned debug) :
        MacroExecutor(macroprocessor, debug)
    {}

    bool ConditionalExecutor::applyMacro(Node& node)
    {
        Node& first = node.list()[0];

        if (first.keyword() == Keyword::If)
        {
            Node cond = node.list()[1];
            Node temp = evaluate(cond, /* is_not_body */ true);
            Node if_true = node.list()[2];
            Node if_false = node.constList().size() > 3 ? node.list()[3] : Node::NilNode;

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

            if (node.nodeType() == NodeType::Macro)
                registerMacro(node);

            return true;
        }

        return false;
    }

    bool ConditionalExecutor::canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Macro && node.list()[0].nodeType() == NodeType::Keyword;
    }
}
