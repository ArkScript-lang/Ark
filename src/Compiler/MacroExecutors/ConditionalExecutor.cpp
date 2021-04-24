#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>
#include <Ark/Log.hpp>
namespace Ark::internal {
    void ConditionalExecutor::execute(
        std::function<Node*(const std::string& name)> find_nearest_macro, 
        std::function<void(Node &node)> registerMacro,
        std::function<bool(const Node& node)> isTruthy,
        std::function<Node(Node& node, bool is_not_body)> evaluate,
        Node &node) {
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
