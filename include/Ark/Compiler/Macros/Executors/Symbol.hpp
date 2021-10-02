/**
 * @file Symbol.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for Symbol Macros
 * @version 0.3
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MACROS_EXECUTORS_SYMBOL_HPP
#define MACROS_EXECUTORS_SYMBOL_HPP

#include <Ark/Compiler/Macros/Executor.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles Symbol macros
     * 
     */
    class SymbolExecutor : public MacroExecutor
    {
    public:
        using MacroExecutor::MacroExecutor;

        bool applyMacro(Node& node) override;
        bool canHandle(Node& node) override;
    };

}

#endif
