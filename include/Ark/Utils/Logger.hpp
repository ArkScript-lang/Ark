/**
 * @file Logger.hpp
 * @author Lexy Plateau (lexplt@gmail.com)
 * @brief Internal logger
 * @date 2024-08-30
 *
 * @copyright Copyright (c) 2024-2026
 */
#ifndef ARK_LOGGER_HPP
#define ARK_LOGGER_HPP

#include <iostream>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <string>
#include <chrono>
#include <source_location>
#include <vector>
#include <unordered_map>

#include <Ark/Utils/Platform.hpp>

namespace Ark::internal
{
    class ARK_API Logger
    {
    public:
        struct MessageAndLocation
        {
            std::string_view message;
            std::source_location location;

            template <typename T>
            // cppcheck-suppress noExplicitConstructor ; we actually want string_views to be casted automatically as MessageAndLocation
            MessageAndLocation(T&& msg, const std::source_location loc = std::source_location::current()) :
                message { std::forward<T>(msg) }, location { loc }
            {}
        };

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
                fmt::println(
                    "{} [{}] {}",
                    fmt::styled("INFO ", fmt::fg(fmt::color::cornflower_blue)),
                    fmt::styled(m_name, fmt::fg(m_pass_color)),
                    fmt::vformat(fmt, fmt::make_format_args(args...)));
        }

        /**
         * @brief Write a warn level log using fmtlib
         * @tparam Args
         * @param fmt format string
         * @param args
         */
        template <typename... Args>
        void warn(const char* fmt, Args&&... args)
        {
            fmt::println(
                std::cerr,
                "{}: {}",
                fmt::styled("Warning", fmt::fg(fmt::color::dark_orange)),
                fmt::vformat(fmt, fmt::make_format_args(args...)));
        }

        /**
         * @brief Write a debug level log using fmtlib
         * @tparam Args
         * @param data format string
         * @param args
         */
        template <typename... Args>
        void debug(const Logger::MessageAndLocation& data, Args&&... args)
        {
            if (shouldDebug())
                fmt::println(
                    "{} [{}] {}({}:{}) {}",
                    fmt::styled("DEBUG", fmt::fg(fmt::color::pale_violet_red)),
                    fmt::styled(m_name, fmt::fg(m_pass_color)),
                    fmt::styled(data.location.file_name(), fmt::fg(fmt::color::pale_turquoise)),
                    fmt::styled(data.location.line(), fmt::fg(fmt::color::pale_turquoise)),
                    fmt::styled(data.location.column(), fmt::fg(fmt::color::pale_turquoise)),
                    fmt::vformat(data.message, fmt::make_format_args(args...)));
        }

        inline void traceStart(std::string&& trace_name)
        {
            m_trace_starts[trace_name] = std::chrono::high_resolution_clock::now();
            m_active_traces.push_back(trace_name);
        }

        inline void traceEnd()
        {
            std::string trace_name = m_active_traces.back();
            m_active_traces.pop_back();

            const auto time = std::chrono::high_resolution_clock::now();
            const std::chrono::duration<double, std::milli> ms_double = time - m_trace_starts[trace_name];
            trace("{} took {:.3f}ms", trace_name, ms_double.count());
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
                fmt::println(
                    "{} [{}] {}",
                    fmt::styled("TRACE", fmt::fg(fmt::color::golden_rod)),
                    fmt::styled(m_name, fmt::fg(m_pass_color)),
                    fmt::vformat(fmt, fmt::make_format_args(args...)));
        }

    private:
        unsigned m_debug;
        std::string m_name;
        fmt::color m_pass_color;
        std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> m_trace_starts;
        std::vector<std::string> m_active_traces;
    };
}

#endif  // ARK_LOGGER_HPP
