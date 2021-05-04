#ifndef ARK_COMPILER_CONDITIONALEXECUTOR_HPP
#define ARK_COMPILER_CONDITIONALEXECUTOR_HPP
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles Conditional macros
     * 
    */
    class ConditionalExecutor : public MacroExecutor
    {
    public:
        void execute(Node &node);
    };

}

#endif