#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(
        MacroProcessor *macroprocessor,
        std::vector<std::shared_ptr<MacroExecutor>> executors
    ) : m_macroprocessor(macroprocessor), m_executors(executors)
    {
        MacroExecutor::init(m_macroprocessor);
    }
    void MacroExecutorPipeline::execute(Node &node)
    {
        for (std::shared_ptr<MacroExecutor> executor : m_executors)
        {
            executor->execute(node);
        }
    }
}