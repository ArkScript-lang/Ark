#ifndef ARK_COMPILER_SYMBOLEXECUTOR_HPP
#define ARK_COMPILER_SYMBOLEXECUTOR_HPP   
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal 
{
    class SymbolExecutor : public MacroExecutor
    {
        public:
            void execute(std::function<Node*(const std::string& name)> const& find_nearest_macro, 
                            std::function<void(Node &node)> const& registerMacro,
                            std::function<bool(const Node& node)> const& isTruthy,
                            std::function<Node(Node& node, bool is_not_body)> const& evaluate,
                            std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> const& apply_to,
                            std::function<void(const std::string& message, const Node& node)> const& throwMacroProcessingError,                        
                            std::function<void(Node& node)> const& func_execute,
                            Node &node);
    };

}

#endif