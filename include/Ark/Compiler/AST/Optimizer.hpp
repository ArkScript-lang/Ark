/**
 * @file Optimizer.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Optimizes a given ArkScript AST
 * @date 2024-07-09
 *
 * @copyright Copyright (c) 2020-2025
 *
 */

#ifndef COMPILER_AST_OPTIMIZER_HPP
#define COMPILER_AST_OPTIMIZER_HPP

#include <functional>
#include <unordered_map>
#include <string>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief The ArkScript AST optimizer
     *
     */
    class ARK_API Optimizer final : public Pass
    {
    public:
        /**
         * @brief Construct a new Optimizer
         *
         * @param debug level of debug
         */
        explicit Optimizer(unsigned debug) noexcept;

        /**
         * @brief Send the AST to the optimizer, then run the different optimization strategies on it
         *
         * @param ast
         */
        void process(const Node& ast) override;

        /**
         * @brief Returns the modified AST
         *
         * @return const Node&
         */
        [[nodiscard]] const Node& ast() const noexcept override;

    private:
        Node m_ast;
        std::unordered_map<std::string, unsigned> m_sym_appearances;

        /**
         * @brief Count the occurrences of each symbol in the AST, recursively, and prune if false/true, while false/true
         *
         * @param node
         */
        void countAndPruneDeadCode(Node& node);

        /**
         * @brief Remove unused global variables from the AST
         *
         * @param node
         */
        void pruneUnusedGlobalVariables(Node& node);
    };
}

#endif
