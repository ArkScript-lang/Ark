#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(
        const std::vector<std::shared_ptr<MacroExecutor>>& executors) : m_executors(executors)
    {

    }
    void MacroExecutorPipeline::m_execute(Node& node)
    {
        for (const std::shared_ptr<MacroExecutor>& executor : m_executors)
        {
            if(executor->m_canHandle(node))
            {
                executor->m_execute(node);
                break;
            }
        }
    }
}