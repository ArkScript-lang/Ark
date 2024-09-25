/**
 * @file Conditional.hpp
 * @author Ray John Alovera (rakista112@gmail.com), Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Executor for Conditional Macros
 * @version 1.1
 * @date 2021-05-04
 *
 * @copyright Copyright (c) 2021-2024
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
    class ConditionalExecutor final : public MacroExecutor
    {
    public:
        using MacroExecutor::MacroExecutor;

        /**
         *
         * @param node
         * @param depth depth of the macro processor evalution
         * @return true if the applying worked
         */
        bool applyMacro(Node& node, unsigned depth) override;

        /**
         *
         * @param node
         * @return true if the executor can handle the given node
         */
        [[nodiscard]] bool canHandle(Node& node) override;
    };

}

#endif
