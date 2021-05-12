#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(
        MacroProcessor* macroprocessor,
        const std::vector<std::shared_ptr<MacroExecutor>>& executors) : m_macroprocessor(macroprocessor), m_executors(executors)
    {
        MacroExecutor::init(m_macroprocessor);
    }
    void MacroExecutorPipeline::execute(Node& node)
    {
        for (const std::shared_ptr<MacroExecutor>& executor : m_executors)
        {
            if(executor->canHandle(node))
            {
                executor->execute(node);
            }
        }
    }
}