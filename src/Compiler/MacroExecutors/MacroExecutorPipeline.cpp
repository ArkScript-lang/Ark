#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(const std::vector<std::shared_ptr<MacroExecutor>>& executors) :
        m_executors(executors)
    {}

    void MacroExecutorPipeline::applyMacro(Node& node)
    {
        for (const std::shared_ptr<MacroExecutor>& executor : m_executors)
        {
            if (executor->canHandle(node))
            {
                executor->applyMacro(node);
                break;
            }
        }
    }
}