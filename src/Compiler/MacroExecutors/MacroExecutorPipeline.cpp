#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(
        std::function<Node *(const std::string &name)> const& find_nearest_macro,
        std::function<void(Node &node)> const& registerMacro,
        std::function<bool(const Node &node)> const& isTruthy,
        std::function<Node(Node &node, bool is_not_body)> const& evaluate,
        std::function<void(const std::unordered_map<std::string, Node> &, Node &, Node *)> const& apply_to,
        std::function<void(const std::string &message, const Node &node)> const& throwMacroProcessingError,
        std::function<void(Node &node)> const& func_execute,
        std::vector<std::shared_ptr<MacroExecutor>> executors
    ) : m_find_nearest_macro(find_nearest_macro), m_registerMacro(registerMacro),
        m_isTruthy(isTruthy), m_evaluate(evaluate), m_apply_to(apply_to), m_throwMacroProcessingError(throwMacroProcessingError),
        m_func_execute(func_execute)
    {
       m_executors = executors;
    }
    void MacroExecutorPipeline::execute(Node &node)
    {
        for (std::shared_ptr<MacroExecutor> executor : m_executors)
        {
            executor->execute(
                m_find_nearest_macro,
                m_registerMacro,
                m_isTruthy,
                m_evaluate,
                m_apply_to,
                m_throwMacroProcessingError,
                m_func_execute,
                node);
        }
    }
}