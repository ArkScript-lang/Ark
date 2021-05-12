#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>

namespace Ark::internal
{
    ConditionalExecutor::ConditionalExecutor(MacroProcessor* macroprocessor, int debug) : MacroExecutor(macroprocessor, debug)
    {
        
    }

    void ConditionalExecutor::m_execute(Node& node)
    {
        Node& first = node.list()[0];

        if (first.keyword() == Keyword::If)
        {
            Node cond = node.list()[1];
            Node temp = m_evaluate(cond, /* is_not_body */ true);
            Node if_true = node.list()[2];
            Node if_false = node.const_list().size() > 2 ? node.list()[3] : Node::NilNode;

            // evaluate cond
            if (m_isTruthy(temp))
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
                m_registerMacro(node);
        }
    }

    bool ConditionalExecutor::m_canHandle(Node& node)
    {
        return node.nodeType() == NodeType::Macro && node.list()[0].nodeType() == NodeType::Keyword;
    }
}
