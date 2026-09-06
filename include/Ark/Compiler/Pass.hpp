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
#include <Ark/Compiler/Statistics.hpp>

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
         * @param stats_collector optional statistics collector
         */
        Pass(std::string name, unsigned debug_level, Statistics* stats_collector = nullptr);

        virtual ~Pass() = default;

        /**
         * @brief Set a custom output stream for the logger
         *
         * @param os output stream
         */
        void configureLogger(std::ostream& os);

    protected:
        Logger m_logger;

        /**
         * @brief Register an event with the number of times it happened
         *
         * @param name
         * @param quantity
         */
        void addStat(Stats name, long quantity) const;

        /**
         * @brief Increase the number of times an event happened by `delta`
         *
         * @param name
         * @param delta default: 1
         */
        void statIncrementCount(Stats name, long delta = 1) const;

        /**
         * @brief Register an event with the time it took
         *
         * @param name
         * @param quantity
         */
        void addStat(const std::string& name, std::chrono::nanoseconds quantity) const;

    private:
        Statistics* m_stats { nullptr };
    };
}

#endif  // ARK_COMPILER_PASS_HPP
