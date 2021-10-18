/**
 * @file Optimizer.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Optimizes a given ArkScript AST
 * @version 0.3
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
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
#include <Ark/Constants.hpp>
#include <Ark/Compiler/AST/makeErrorCtx.hpp>

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
         */
        explicit Optimizer(uint16_t options) noexcept;

        /**
         * @brief Send the AST to the optimizer, then run the different optimization strategies on it
         * 
         * @param ast 
         */
        void feed(const Node& ast);

        /**
         * @brief Returns the modified AST
         * 
         * @return const Node& 
         */
        const Node& ast() const noexcept;

    private:
        Node m_ast;
        uint16_t m_options;
        std::unordered_map<std::string, unsigned> m_sym_appearances;

        /**
         * @brief Generate a fancy error message
         * 
         * @param message 
         * @param node 
         */
        [[noreturn]] void throwOptimizerError(const std::string& message, const Node& node);

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
        void runOnGlobalScopeVars(Node& node, const std::function<void(Node&, Node&, int)>& func);

        /**
         * @brief Count the occurences of each symbol in the AST, recursively
         * 
         * @param node 
         */
        void countOccurences(Node& node);
    };
}

#endif
