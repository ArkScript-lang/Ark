#ifndef ARK_COMPILER_EXECUTORPIPELINE_HPP
#define ARK_COMPILER_EXECUTORPIPELINE_HPP   
#include <vector>
#include <Ark/Compiler/MacroExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/SymbolExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/ListExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>

namespace Ark::internal 
{
    class MacroExecutorPipeline 
    {
        std::function<Node*(const std::string& name)> find_nearest_macro;
        std::function<void(Node &node)> registerMacro;
        std::function<bool(const Node& node)> isTruthy;
        std::function<Node(Node& node, bool is_not_body)> evaluate;
        std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> apply_to;
        std::function<void(const std::string& message, const Node& node)> throwMacroProcessingError;                       
        std::function<void(Node& node)> func_execute;
        std::vector<std::shared_ptr<MacroExecutor>> m_executors = std::vector<std::shared_ptr<MacroExecutor>>();

    public:
        MacroExecutorPipeline() {}
        MacroExecutorPipeline(
            std::function<Node*(const std::string& name)> find_nearest_macro, 
                            std::function<void(Node &node)> registerMacro,
                            std::function<bool(const Node& node)> isTruthy,
                            std::function<Node(Node& node, bool is_not_body)> evaluate,
                            std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> apply_to,
                            std::function<void(const std::string& message, const Node& node)> throwMacroProcessingError,                        
                            std::function<void(Node& node)> func_execute,
                            std::vector<std::shared_ptr<MacroExecutor>> executors
        );
        void execute(Node &node);

    };
}

#endif