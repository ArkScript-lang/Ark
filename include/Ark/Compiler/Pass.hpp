/**
 * @file Pass.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Interface for a compiler pass (take in an AST, output an AST)
 * @version 1.0
 * @date 2024-07-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef ARK_COMPILER_PASS_HPP
#define ARK_COMPILER_PASS_HPP

#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Logger.hpp>

namespace Ark::internal
{
    /**
     * @brief An interface to describe compiler passes
     */
    class Pass
    {
    public:
        /**
         * @brief Construct a new Pass object
         */
        Pass();

        /**
         * @brief Construct a new Pass object
         *
         * @param name the pass name, used for logging
         * @param debug_level debug level
         */
        Pass(std::string name, unsigned debug_level);

        virtual ~Pass() = default;

        /**
         * @brief Start processing the given AST
         * @param ast
         */
        virtual void process(const Node& ast) = 0;

        /**
         * @brief Output of the compiler pass
         *
         * @return const Node& the modified AST
         */
        [[nodiscard]] virtual const Node& ast() const noexcept = 0;

    protected:
        Logger m_logger;
    };
}

#endif  // ARK_COMPILER_PASS_HPP
