#ifndef ARK_COMPILER_SYMBOLEXECUTOR_HPP
#define ARK_COMPILER_SYMBOLEXECUTOR_HPP
#include <Ark/Compiler/MacroExecutor.hpp>
#include <unordered_map>
#include <vector>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles Symbol macros
     * 
     */
    class SymbolExecutor : public MacroExecutor
    {
    public:
        void execute(Node& node) override;
    };

}

#endif