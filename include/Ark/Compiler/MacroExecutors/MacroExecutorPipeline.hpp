#ifndef ARK_COMPILER_EXECUTORPIPELINE_HPP
#define ARK_COMPILER_EXECUTORPIPELINE_HPP
#include <vector>
#include <Ark/Compiler/MacroExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/SymbolExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/ListExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/ConditionalExecutor.hpp>

namespace Ark::internal
{
    class MacroProcessor;

    /**
     * @brief The class that initializes the MacroExecutors 
     * 
     */
    class MacroExecutorPipeline
    {
        MacroProcessor *m_macroprocessor;
        std::vector<std::shared_ptr<MacroExecutor>> m_executors;

    public:
        MacroExecutorPipeline(
            MacroProcessor *macroprocessor,
            std::vector<std::shared_ptr<MacroExecutor>> executors);
        /**
         * @brief Passes node through all MacroExecutors sequentially
         * 
         * @param node node on which to operate
         */
        void execute(Node &node);
    };
}

#endif