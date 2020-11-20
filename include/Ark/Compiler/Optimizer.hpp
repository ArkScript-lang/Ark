/**
 * @file Optimizer.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Optimizes a given ArkScript AST
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ark_compiler_optimizer
#define ark_compiler_optimizer

#include <functional>
#include <unordered_map>
#include <string>
#include <cinttypes>

#include <Ark/Compiler/Node.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Constants.hpp>

namespace Ark
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
        Optimizer(uint16_t options) noexcept;

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
        internal::Node m_ast;
        uint16_t m_options;
        std::unordered_map<std::string, unsigned> m_symAppearances;

        /**
         * @brief Generate a fancy error message
         * 
         * @param message 
         * @param node 
         */
        inline void throwOptimizerError(const std::string& message, const internal::Node& node);

        // iterate over the AST and remove unused top level functions and constants
        void remove_unused();
        void run_on_global_scope_vars(internal::Node& node, const std::function<void(internal::Node&, internal::Node&, int)>& func);
        void count_occurences(const internal::Node& node);
    };
}

#endif  // ark_compiler_optimizer