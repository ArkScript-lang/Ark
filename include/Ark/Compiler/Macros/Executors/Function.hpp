/**
 * @file Function.hpp
 * @author Ray John Alovera (rakista112@gmail.com), Lex Plateau (lexplt.dev@gmail.com)
 * @brief Executor for List Macros
 * @date 2021-05-04
 *
 * @copyright Copyright (c) 2021-2026
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

        /**
         *
         * @param node
         * @param depth depth of the macro processor evaluation
         * @return true if the applying worked
         */
        bool applyMacro(Node& node, unsigned depth) override;

        /**
         *
         * @param node
         * @return true if the executor can handle the given node
         */
        [[nodiscard]] bool canHandle(Node& node) override;

        [[nodiscard]] Node macroNode(Node& node) override;

    private:
        void unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, std::size_t index = 0, std::size_t unify_depth = 0);
    };

}

#endif
