#include <Ark/Compiler/Macros/Pipeline.hpp>

namespace Ark::internal
{
    MacroExecutorPipeline::MacroExecutorPipeline(const std::vector<std::shared_ptr<MacroExecutor>>& executors) :
        m_executors(executors)
    {}

    bool MacroExecutorPipeline::applyMacro(Node& node) const
    {
        for (const std::shared_ptr<MacroExecutor>& executor : m_executors)
        {
            if (executor->canHandle(node))
            {
                if (executor->applyMacro(node))
                    return true;
            }
        }
        return false;
    }
}
