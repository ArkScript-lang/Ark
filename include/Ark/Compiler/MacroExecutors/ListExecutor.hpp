/**
 * @file ListExecutor.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for List Macros
 * @version 0.1
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
        ListExecutor(MacroProcessor* macroprocessor, int debug = 0);
        void m_execute(Node &node) override;
        bool m_canHandle(Node &node) override;
    };

}

#endif