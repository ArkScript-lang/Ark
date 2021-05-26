/**
 * @file ConditionalExecutor.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for Conditional Macros
 * @version 0.3
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_CONDITIONALEXECUTOR_HPP
#define ARK_COMPILER_CONDITIONALEXECUTOR_HPP

#include <Ark/Compiler/MacroExecutor.hpp>
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
        ConditionalExecutor(MacroProcessor* macroprocessor, unsigned debug = 0);

        void applyMacro(Node& node) override;
        bool canHandle(Node& node) override;
    };

}

#endif