/**
 * @file Pass.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Interface for a compiler pass (take in an AST, output an AST)
 * @version 0.1
 * @date 2024-07-21
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef ARK_COMPILER_PASS_HPP
#define ARK_COMPILER_PASS_HPP

#include <Ark/Compiler/AST/Node.hpp>

#include <iostream>
#include <fmt/format.h>

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

        virtual void process(const Node& ast) = 0;

        /**
         * @brief Output of the compiler pass
         *
         * @return const Node& the modified AST
         */
        [[nodiscard]] virtual const Node& ast() const noexcept = 0;

        enum class LogLevel
        {
            None,
            Info,
            Debug,
            Trace,
            Other
        };

    protected:
        inline unsigned debugLevel() const { return m_debug; }

        inline bool shouldInfo() const { return m_debug >= 1; }
        inline bool shouldDebug() const { return m_debug >= 2; }
        inline bool shouldTrace() const { return m_debug >= 3; }

        template <typename... Args>
        void log(const char* fmt, Args&&... args)
        {
            if (shouldInfo())
                std::cout << fmt::format("[INFO ][{}] ", m_name)
                          << fmt::vformat(fmt, fmt::make_format_args(args...))
                          << std::endl;
        }

        template <typename... Args>
        void logDebug(const char* fmt, Args&&... args)
        {
            if (shouldDebug())
                std::cout << fmt::format("[DEBUG][{}] ", m_name)
                          << fmt::vformat(fmt, fmt::make_format_args(args...))
                          << std::endl;
        }

        template <typename... Args>
        void logTrace(const char* fmt, Args&&... args)
        {
            if (shouldTrace())
                std::cout << fmt::format("[TRACE][{}] ", m_name)
                          << fmt::vformat(fmt, fmt::make_format_args(args...))
                          << std::endl;
        }

    private:
        unsigned m_debug;
        std::string m_name;
    };
}

#endif  // ARK_COMPILER_PASS_HPP
