/**
 * @file Pipeline.hpp
 * @author Ray John Alovera (rakista112@gmail.com), Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The Chain of Responsibility class for running nodes through MacroExecutors
 * @version 0.4
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef COMPILER_MACROS_PIPELINE_HPP
#define COMPILER_MACROS_PIPELINE_HPP

#include <vector>
#include <memory>

#include <Ark/Compiler/Macros/Executor.hpp>

namespace Ark::internal
{
    class MacroProcessor;

    /**
     * @brief The class that initializes the MacroExecutors 
     * 
     */
    class MacroExecutorPipeline
    {
    public:
        MacroExecutorPipeline() = default;

        /**
         * @brief Construct a new Macro Executor Pipeline object
         * 
         * @param executors 
         */
        explicit MacroExecutorPipeline(const std::vector<std::shared_ptr<MacroExecutor>>& executors);

        /**
         * @brief Passes node through all MacroExecutors sequentially
         * 
         * @param node node on which to operate
         * @return true if a macro was applied
         * @return false 
         */
        bool applyMacro(Node& node);

    private:
        std::vector<std::shared_ptr<MacroExecutor>> m_executors;
    };
}

#endif
