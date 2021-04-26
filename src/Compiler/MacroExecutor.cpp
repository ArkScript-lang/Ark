#include <Ark/Compiler/MacroExecutor.hpp>

namespace Ark::internal 
{
    MacroExecutor *MacroExecutor::set_next(MacroExecutor *executor)
    {
        m_next_executor = executor;
        return executor;
    }
}