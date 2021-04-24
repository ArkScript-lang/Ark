#include <Ark/Compiler/MacroExecutors/SymbolExecutor.hpp>
#include <Ark/Log.hpp>
namespace Ark::internal {
    void SymbolExecutor::execute(std::function<Node*(const std::string& name)> find_nearest_macro, 
                            std::function<void(Node &node)> registerMacro,
                            std::function<bool(const Node& node)> isTruthy,
                            std::function<Node(Node& node, bool is_not_body)> evaluate,
                            std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> apply_to,
                            std::function<void(const std::string& message, const Node& node)> throwMacroProcessingError,
                            Node &node) {
        // error ?
        Node* macro = find_nearest_macro(node.string());

        if (macro != nullptr)
        {
            if (m_debug >= 3)
                Ark::logger.info("Found macro for", node.string());

            // !{name value}
            if (macro->const_list().size() == 2)
                node = macro->list()[1];
        }

        return;
    }
}
