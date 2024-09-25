/**
 * @file Logger.hpp
 * @author Alexandre Plateau (lexplt@gmail.comhf
 * @brief Internal logger
 * @version 0.1
 * @date 2024-08-30
 *
 * @copyright Copyright (c) 2024
 */
#ifndef ARK_LOGGER_HPP
#define ARK_LOGGER_HPP

#include <iostream>
#include <fmt/format.h>

#include <string>

namespace Ark::internal
{
    enum class LogLevel
    {
        None,
        Info,
        Debug,
        Trace,
        Other
    };

    class Logger
    {
    public:
        /**
         * @brief Construct a new Logger object
         *
         * @param name the pass name, used for logging
         * @param debug_level debug level
         */
        Logger(std::string name, unsigned debug_level);

        [[nodiscard]] inline unsigned debugLevel() const { return m_debug; }

        [[nodiscard]] inline bool shouldInfo() const { return m_debug >= 1; }
        [[nodiscard]] inline bool shouldDebug() const { return m_debug >= 2; }
        [[nodiscard]] inline bool shouldTrace() const { return m_debug >= 3; }

        /**
         * @brief Write an info level log using fmtlib
         * @tparam Args
         * @param fmt format string
         * @param args
         */
        template <typename... Args>
        void info(const char* fmt, Args&&... args)
        {
            if (shouldInfo())
                fmt::println("[INFO ][{}] {}", m_name, fmt::vformat(fmt, fmt::make_format_args(args...)));
        }

        /**
         * @brief Write a debug level log using fmtlib
         * @tparam Args
         * @param fmt format string
         * @param args
         */
        template <typename... Args>
        void debug(const char* fmt, Args&&... args)
        {
            if (shouldDebug())
                fmt::println("[DEBUG][{}] {}", m_name, fmt::vformat(fmt, fmt::make_format_args(args...)));
        }

        /**
         * @brief Write a trace level log using fmtlib
         * @tparam Args
         * @param fmt format string
         * @param args
         */
        template <typename... Args>
        void trace(const char* fmt, Args&&... args)
        {
            if (shouldTrace())
                fmt::println("[TRACE][{}] {}", m_name, fmt::vformat(fmt, fmt::make_format_args(args...)));
        }

    private:
        unsigned m_debug;
        std::string m_name;
    };
}

#endif  // ARK_LOGGER_HPP
