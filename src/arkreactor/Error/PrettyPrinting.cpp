#include <Ark/Error/PrettyPrinting.hpp>

#include <Ark/Constants.hpp>
#include <Ark/Utils/Files.hpp>
#include <Ark/Utils/Utils.hpp>

#include <array>
#include <cassert>

#include <fmt/color.h>
#include <fmt/ostream.h>

namespace Ark::Diagnostics
{
    [[nodiscard]] bool isPairableChar(const char c)
    {
        return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}';
    }

    void colorizeLine(const std::string& line, LineColorContextCounts& ctx, std::ostream& os)
    {
        // clang-format off
        constexpr std::array pairing_color {
            fmt::color::light_blue,
            fmt::color::light_green,
            fmt::color::light_salmon,
            fmt::color::light_yellow,
            fmt::color::light_cyan,
            fmt::color::light_coral
        };
        // clang-format on
        constexpr std::size_t pairing_color_size = pairing_color.size();

        for (const char c : line)
        {
            if (isPairableChar(c))
            {
                int idx = 0;

                switch (c)
                {
                    case '(':
                        idx = ctx.open_parentheses;
                        ctx.open_parentheses++;
                        break;
                    case ')':
                        ctx.open_parentheses--;
                        idx = ctx.open_parentheses;
                        break;
                    case '[':
                        idx = ctx.open_square_braces;
                        ctx.open_square_braces++;
                        break;
                    case ']':
                        ctx.open_square_braces--;
                        idx = ctx.open_square_braces;
                        break;
                    case '{':
                        idx = ctx.open_curly_braces;
                        ctx.open_curly_braces++;
                        break;
                    case '}':
                        ctx.open_curly_braces--;
                        idx = ctx.open_curly_braces;
                        break;
                    default:
                        break;
                }

                const std::size_t pairing_color_index = static_cast<std::size_t>(std::abs(idx)) % pairing_color_size;
                fmt::print(os, "{}", fmt::styled(c, fmt::fg(pairing_color[pairing_color_index])));
            }
            else
                fmt::print(os, "{}", c);
        }
    }

    Printer::Printer(
        const std::string& filename, const std::size_t target_line,
        const std::optional<std::size_t> end_target_line, const bool colorize) :
        m_should_colorize(colorize)
    {
        const std::string code = filename == ARK_NO_NAME_FILE ? "" : Utils::readFile(filename);
        m_source = Utils::splitString(code, '\n');

        m_window = Window(target_line, end_target_line.value_or(target_line), m_source.size());
        m_current_line = m_window.start;
    }

    std::string Printer::sliceCode(const internal::FilePos start, const std::optional<internal::FilePos>& end) const
    {
        std::string code;
        if (!end)
        {
            code = m_source[start.line];
            Utils::ltrim(Utils::rtrim(code));
        }
        else
        {
            if (start.line == end->line)
                code = m_source[start.line].substr(start.column, end->column);
            else
            {
                code = m_source[start.line].substr(start.column);

                if (end->line - start.line > 1)
                {
                    for (std::size_t i = start.line + 1; i <= end->line - 1; ++i)
                    {
                        code += "\n";
                        code += m_source[i];
                    }
                }
                code += "\n" + m_source[end->line].substr(0, end->column);
            }
        }

        return code;
    }

    void Printer::extendWindow(const std::size_t line_to_include)
    {
        // showing the context will require an ellipsis, to avoid showing too many lines in the error message
        if (line_to_include + 3 < m_window.start)
            m_window.skip_start_at = line_to_include + 3;
        m_window.resume_at = m_window.start;

        assert(line_to_include <= m_window.start && "line_to_include has to be before the start of our base context, source of errors are always before our errors");
        // due to how context works, if it points to the same file,
        // we are guaranteed it will be before our error
        m_window.start = line_to_include >= 3 ? line_to_include - 3 : 0;
        m_current_line = m_window.start;
    }

    void Printer::printLine(std::ostream& os)
    {
        if (!hasContent())
            return;

        if (m_window.hasSkip() &&
            m_current_line >= m_window.skip_start_at.value() &&
            m_current_line < m_window.resume_at.value())
        {
            // do not print the current line, we want to skip 1 or more lines
            ++m_current_line;
            return;
        }

        // show current line with its number
        fmt::print(os, "{: >5} |{}", m_current_line + 1, !m_source[m_current_line].empty() ? " " : "");
        if (m_should_colorize)
            colorizeLine(m_source[m_current_line], m_color_ctx, os);
        else
            fmt::print(os, "{}", m_source[m_current_line]);
        fmt::print(os, "\n");

        ++m_current_line;

        // if skip_start_at is equal to the next line to print, and we have to skip,
        // display an ellipsis
        if (m_window.skip_start_at &&
            m_current_line == m_window.skip_start_at.value())
            fmt::print(os, "  ... |\n");
    }

    bool Printer::isTargetLine() const
    {
        return m_window.target + 1 <= m_current_line && m_current_line <= m_window.target_end + 1;
    }

    bool Printer::hasContent() const
    {
        return m_current_line < m_window.end && !m_source.empty() && m_window.target < m_window.end;
    }

    bool Printer::coversLine(const std::size_t line_number) const
    {
        return m_window.start <= line_number && line_number < m_window.end;
    }

}
