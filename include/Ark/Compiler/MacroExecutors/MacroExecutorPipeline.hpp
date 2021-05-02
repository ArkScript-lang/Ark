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
        std::function<Node*(const std::string& name)> const m_find_nearest_macro;
        std::function<void(Node &node)> const m_registerMacro;
        std::function<bool(const Node& node)> const m_isTruthy;
        std::function<Node(Node& node, bool is_not_body)> const m_evaluate;
        std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> const m_apply_to;
        std::function<void(const std::string& message, const Node& node)> const m_throwMacroProcessingError;                       
        std::function<void(Node& node)> const m_func_execute;
        std::vector<std::shared_ptr<MacroExecutor>> m_executors;

    public:
        MacroExecutorPipeline(
            std::function<Node*(const std::string& name)> const& find_nearest_macro, 
            std::function<void(Node &node)> const& registerMacro,
            std::function<bool(const Node& node)> const& isTruthy,
            std::function<Node(Node& node, bool is_not_body)> const& evaluate,
            std::function<void(const std::unordered_map<std::string, Node>&, Node&, Node*)> const& apply_to,
            std::function<void(const std::string& message, const Node& node)> const& throwMacroProcessingError,                        
            std::function<void(Node& node)> const& func_execute,
            std::vector<std::shared_ptr<MacroExecutor>> executors
        );
        void execute(Node &node);

    };
}

#endif