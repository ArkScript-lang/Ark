#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>
#include <Ark/Log.hpp>

namespace Ark::internal
{
    void ConditionalExecutor::execute(Node& node)
    {
        if (node.nodeType() == NodeType::Macro && node.list()[0].nodeType() == NodeType::Keyword)
        {
            Node& first = node.list()[0];

            if (first.keyword() == Keyword::If)
            {
                Node cond = node.list()[1];
                Node temp = evaluate(cond, /* is_not_body */ true);
                Node if_true = node.list()[2];
                Node if_false = node.const_list().size() > 2 ? node.list()[3] : Node::NilNode;

                // evaluate cond
                if (isTruthy(temp))
                    node = if_true;
                else if (node.const_list().size() > 2)
                    node = if_false;
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
    }
}
