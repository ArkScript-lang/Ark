/**
 * @file Optimizer.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Optimizes a given ArkScript AST
 * @version 1.0
 * @date 2024-07-09
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef COMPILER_AST_OPTIMIZER_HPP
#define COMPILER_AST_OPTIMIZER_HPP

#include <functional>
#include <unordered_map>
#include <string>
#include <cinttypes>

#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Exceptions.hpp>

namespace Ark::internal
{
    /**
     * @brief The ArkScript AST optimizer
     *
     */
    class Optimizer
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
        void process(const Node& ast);

        /**
         * @brief Returns the modified AST
         *
         * @return const Node&
         */
        [[nodiscard]] const Node& ast() const noexcept;

    private:
        Node m_ast;
        unsigned m_debug;
        std::unordered_map<std::string, unsigned> m_sym_appearances;

        /**
         * @brief Generate a fancy error message
         *
         * @param message
         * @param node
         */
        [[noreturn]] static void throwOptimizerError(const std::string& message, const Node& node);

        /**
         * @brief Iterate over the AST and remove unused top level functions and constants
         *
         */
        void remove_unused();

        /**
         * @brief Run a given functor on the global scope symbols
         *
         * @param node
         * @param func
         */
        void runOnGlobalScopeVars(Node& node, const std::function<void(Node&, Node&, std::size_t)>& func);

        /**
         * @brief Count the occurrences of each symbol in the AST, recursively
         *
         * @param node
         */
        void countOccurences(Node& node);
    };
}

#endif
