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

#ifndef ARK_COMPILER_OPTIMIZER_HPP
#define ARK_COMPILER_OPTIMIZER_HPP

#include <functional>
#include <unordered_map>
#include <string>
#include <cinttypes>

#include <Ark/Compiler/Node.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Constants.hpp>
#include <Ark/Compiler/makeErrorCtx.hpp>

namespace Ark
{
    namespace internal
    {
        /**
         * @brief Category for complex nodes (not including values)
         * 
         */
        enum class NodeCategory
        {
            Store,  ///< This comprises let, mut and set nodes
            If,
            Function,
            FunctionCall,
            While
        };
    }

    /**
     * @brief The ArkScript AST optimizer
     * 
     */
    class Optimizer
    {
    public:
        using flag_t = uint32_t;

        /**
         * @brief Construct a new Optimizer
         * 
         */
        explicit Optimizer(unsigned debug, uint16_t options) noexcept;

        /**
         * @brief Send the AST to the optimizer, then run the different optimization strategies on it
         * 
         * @param ast 
         */
        void feed(const internal::Node& ast);

        /**
         * @brief Returns the modified AST
         * 
         * @return const internal::Node& 
         */
        const internal::Node& ast() const noexcept;

    private:
        internal::Node m_ast;  ///< Used to store our modified version of the AST
        unsigned m_debug;
        uint16_t m_options;
        std::vector<flag_t> m_flags;
        std::unordered_map<std::string, unsigned> m_sym_appearances;

        /**
         * @brief Generate a fancy error message
         * 
         * @param message 
         * @param node 
         */
        inline void throwOptimizerError(const std::string& message, const internal::Node& node)
        {
            throw OptimizerError(internal::makeNodeBasedErrorCtx(message, node));
        }

        void visit(internal::Node& node, std::size_t depth, flag_t flags = 0);

        flag_t determineFlags(const internal::Node& node, std::size_t child_pos);

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
        void runOnGlobalScopeVars(internal::Node& node, const std::function<void(internal::Node&, internal::Node&, int)>& func);

        /**
         * @brief Count the occurences of each symbol in the AST, recursively
         * 
         * @param node 
         */
        void countOccurences(internal::Node& node);
    };
}

#endif
