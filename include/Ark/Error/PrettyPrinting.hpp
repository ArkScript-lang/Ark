/**
 * @file PrettyPrinting.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Pretty printing utilities for diagnostics
 * @date 2025-08-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef ARK_ERROR_PRETTYPRINTING_HPP
#define ARK_ERROR_PRETTYPRINTING_HPP

#include <string>
#include <ostream>
#include <optional>

#include <Ark/Utils/Platform.hpp>

namespace Ark::Diagnostics
{
    struct LineColorContextCounts
    {
        int open_parentheses = 0;
        int open_square_braces = 0;
        int open_curly_braces = 0;
    };

    struct ARK_API Window
    {
        std::size_t start;  ///< First line number to display
        std::size_t target;
        std::size_t end;  ///< Last line of the context, not displayed

        std::optional<std::size_t> skip_start_at = std::nullopt;
        std::optional<std::size_t> resume_at = std::nullopt;

        Window() :
            start(0), target(0), end(0)
        {}

        Window(const std::size_t target_line, const std::size_t line_count) :
            target(target_line)
        {
            start = target_line >= 3 ? target_line - 3 : 0;
            end = target_line + 3 <= line_count ? target_line + 3 : line_count;
        }

        [[nodiscard]] bool hasSkip() const
        {
            return skip_start_at.has_value() && resume_at.has_value();
        }
    };

    /**
     * @brief Source printer for diagnostics
     */
    class ARK_API Printer
    {
    public:
        /**
         * @brief Create a new Printer object
         *
         * @param filename path to the file that has an error
         * @param target_line line of the error (0-indexed)
         * @param colorize if we should colorize the output or not
         */
        Printer(const std::string& filename, std::size_t target_line, bool colorize);

        /**
         * @brief Extend the window of lines to show, to include a given line.
         *        Useful to display the origin of an error.
         *
         * @param line_to_include line to include (0-indexed)
         */
        void extendWindow(std::size_t line_to_include);

        void extendWindowEnd();

        /**
         * @brief Print the current line and advance by one
         *
         * @param os output stream
         */
        void printLine(std::ostream& os);

        /**
         * @brief Check if we printed the target line
         *
         * @return true if the last displayed line is the target line
         * @return false otherwise
         */
        [[nodiscard]] bool isTargetLine() const;

        /**
         * @brief Check if there are lines to print
         *
         * @return true while the window isn't exhausted
         * @return false when there is nothing more to print
         */
        [[nodiscard]] bool hasContent() const;

        [[nodiscard]] bool coversLine(std::size_t line_number) const;

        [[nodiscard]] inline const Window& window() const
        {
            return m_window;
        }

        // fixme
        [[nodiscard]] inline std::size_t current() const
        {
            return m_current_line;
        }

        [[nodiscard]] inline const std::string& currentLine() const
        {
            return m_source[m_current_line];
        }

        [[nodiscard]] inline const std::string& targetLine() const
        {
            return m_source[m_window.target];
        }

    private:
        bool m_should_colorize;
        std::vector<std::string> m_source;
        Window m_window;
        std::size_t m_current_line;
        LineColorContextCounts m_color_ctx;
    };
}

#endif  // ARK_ERROR_PRETTYPRINTING_HPP
