#ifndef ARK_COMPILER_LISTEXECUTOR_HPP
#define ARK_COMPILER_LISTEXECUTOR_HPP   
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>
namespace Ark::internal {
    class ListExecutor : public MacroExecutor{
        public:
            void execute(std::function<Node*(const std::string& name)> find_nearest_macro, 
                                std::function<void(Node &node)> registerMacro,
                                std::function<bool(const Node& node)> isTruthy,
                                std::function<Node(Node& node, bool is_not_body)> evaluate,
                                std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> apply_to,
                                std::function<void(const std::string& message, const Node& node)> processing_error,
                                Node &node);
    };

}

#endif