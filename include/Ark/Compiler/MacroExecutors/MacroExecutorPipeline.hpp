/**
 * @file MacroExecutorPipeline.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief The Chain of Responsibility class for running nodes through MacroExecutors
 * @version 0.1
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_EXECUTORPIPELINE_HPP
#define ARK_COMPILER_EXECUTORPIPELINE_HPP

#include <vector>
#include <Ark/Compiler/MacroExecutor.hpp>

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
        /**
         * @brief Construct a new Macro Executor Pipeline object
         * 
         * @param executors 
         */
        MacroExecutorPipeline(const std::vector<std::shared_ptr<MacroExecutor>>& executors);

        /**
         * @brief Passes node through all MacroExecutors sequentially
         * 
         * @param node node on which to operate
         */
        void execute(Node& node);

    private:
        std::vector<std::shared_ptr<MacroExecutor>> m_executors;
    };
}

#endif