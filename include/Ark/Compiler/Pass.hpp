/**
 * @file Pass.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Interface for a compiler pass
 * @date 2024-07-21
 *
 * @copyright Copyright (c) 2024-2026
 *
 */
#ifndef ARK_COMPILER_PASS_HPP
#define ARK_COMPILER_PASS_HPP

#include <Ark/Utils/Platform.hpp>
#include <Ark/Utils/Logger.hpp>

#include <ostream>

namespace Ark::internal
{
    /**
     * @brief An interface to describe compiler passes
     */
    class ARK_API Pass
    {
    public:
        /**
         * @brief Construct a new Pass object
         *
         * @param name the pass name, used for logging
         * @param debug_level debug level
         */
        Pass(std::string name, unsigned debug_level);

        virtual ~Pass() = default;

        /**
         * @brief Set a custom output stream for the logger
         *
         * @param os output stream
         */
        void configureLogger(std::ostream& os);

    protected:
        Logger m_logger;
    };
}

#endif  // ARK_COMPILER_PASS_HPP
