
#ifndef ARK_COMPILER_EXECUTOR_HPP
#define ARK_COMPILER_EXECUTOR_HPP   

#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    class MacroExecutor {
        protected:
            unsigned int m_debug;
            MacroExecutor *m_next_executor = nullptr;
        public:
            MacroExecutor () : m_debug(0) {};
            MacroExecutor (unsigned int debug) : m_debug(debug) {};
            MacroExecutor *set_next(MacroExecutor *executor);
            virtual void execute(std::function<Node*(const std::string& name)> find_nearest_macro, 
                                    std::function<void(Node &node)> registerMacro,
                                    std::function<bool(const Node& node)> isTruthy,
                                    std::function<Node(Node& node, bool is_not_body)> evaluate,
                                    std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> apply_to,
                                    std::function<void(const std::string& message, const Node& node)> throwMacroProcessingError,
                                    Node &node) = 0;
    };

   
}

#endif