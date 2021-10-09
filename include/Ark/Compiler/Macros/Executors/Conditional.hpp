/**
 * @file Conditional.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for Conditional Macros
 * @version 0.4
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MACROS_EXECUTORS_CONDITIONAL_HPP
#define MACROS_EXECUTORS_CONDITIONAL_HPP

#include <Ark/Compiler/Macros/Executor.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles Conditional macros
     * 
    */
    class ConditionalExecutor : public MacroExecutor
    {
    public:
        using MacroExecutor::MacroExecutor;

        bool applyMacro(Node& node) override;
        bool canHandle(Node& node) override;
    };

}

#endif
