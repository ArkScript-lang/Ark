/**
 * @file ListExecutor.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for List Macros
 * @version 0.2
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_LISTEXECUTOR_HPP
#define ARK_COMPILER_LISTEXECUTOR_HPP

#include <Ark/Compiler/MacroExecutor.hpp>
#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles List macros
     * 
    */
    class ListExecutor : public MacroExecutor
    {
    public:
        ListExecutor(MacroProcessor* macroprocessor, unsigned debug = 0);

        void applyMacro(Node& node) override;
        bool canHandle(Node& node) override;
    };

}

#endif