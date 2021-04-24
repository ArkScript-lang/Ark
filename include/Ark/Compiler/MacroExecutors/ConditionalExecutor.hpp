#ifndef ARK_COMPILER_SYMBOLEXECUTOR_HPP
#define ARK_COMPILER_SYMBOLEXECUTOR_HPP   
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>
namespace Ark::internal {
    class ConditionalExecutor : public MacroExecutor{
        public:
            void execute(std::function<Node*(const std::string& name)> find_nearest_macro, 
                                std::function<void(Node &node)> registerMacro,
                                std::function<bool(const Node& node)> isTruthy,
                                std::function<Node(Node& node, bool is_not_body)> evaluate,
                                Node &node);
    };

}

#endif