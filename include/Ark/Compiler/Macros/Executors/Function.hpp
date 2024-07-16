/**
 * @file Function.hpp
 * @author Ray John Alovera (rakista112@gmail.com), Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Executor for List Macros
 * @version 2.0
 * @date 2021-05-04
 *
 * @copyright Copyright (c) 2021-2024
 *
 */

#ifndef MACROS_EXECUTORS_LIST_HPP
#define MACROS_EXECUTORS_LIST_HPP

#include <Ark/Compiler/Macros/Executor.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Handles function macros
     *
     */
    class FunctionExecutor final : public MacroExecutor
    {
    public:
        using MacroExecutor::MacroExecutor;

        bool applyMacro(Node& node, unsigned depth) override;
        bool canHandle(Node& node) override;
    };

}

#endif
