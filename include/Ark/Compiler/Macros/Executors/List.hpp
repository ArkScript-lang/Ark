/**
 * @file List.hpp
 * @author Ray John Alovera (rakista112@gmail.com)
 * @brief Executor for List Macros
 * @version 0.3
 * @date 2021-05-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MACROS_EXECUTORS_LIST_HPP
#define MACROS_EXECUTORS_LIST_HPP

#include <Ark/Compiler/Macros/Executor.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles List macros
     * 
    */
    class ListExecutor : public MacroExecutor
    {
    public:
        using MacroExecutor::MacroExecutor;

        bool applyMacro(Node& node) override;
        bool canHandle(Node& node) override;
    };

}

#endif
