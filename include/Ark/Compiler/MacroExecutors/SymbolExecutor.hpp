/**
 * @file SymbolExecutor.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for Symbol Macros
 * @version 0.1
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_SYMBOLEXECUTOR_HPP
#define ARK_COMPILER_SYMBOLEXECUTOR_HPP
#include <Ark/Compiler/MacroExecutor.hpp>
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
        SymbolExecutor(MacroProcessor* macroprocessor, int debug = 0);

        void execute(Node &node) override;
        bool canHandle(Node &node) override;
    };

}

#endif