#include <Ark/Error/Exceptions.hpp>

#include <cassert>
#include <sstream>
#include <algorithm>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <Ark/Constants.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Utils/Files.hpp>
#include <Ark/Utils/Literals.hpp>
#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::Diagnostics
{
    struct LineColorContextCounts
    {
        int open_parentheses = 0;
        int open_square_braces = 0;
        int open_curly_braces = 0;
    };

    inline bool isPairableChar(const char c)
    {
        return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}';
    }

    void colorizeLine(const std::string& line, LineColorContextCounts& line_color_context_counts, std::ostream& ss)
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
                std::size_t pairing_color_index = 0;

                switch (c)
                {
                    case '(':
                        pairing_color_index = static_cast<std::size_t>(std::abs(line_color_context_counts.open_parentheses)) % pairing_color_size;
                        line_color_context_counts.open_parentheses++;
                        break;
                    case ')':
                        line_color_context_counts.open_parentheses--;
                        pairing_color_index = static_cast<std::size_t>(std::abs(line_color_context_counts.open_parentheses)) % pairing_color_size;
                        break;
                    case '[':
                        pairing_color_index = static_cast<std::size_t>(std::abs(line_color_context_counts.open_square_braces)) % pairing_color_size;
                        line_color_context_counts.open_square_braces++;
                        break;
                    case ']':
                        line_color_context_counts.open_square_braces--;
                        pairing_color_index = static_cast<std::size_t>(std::abs(line_color_context_counts.open_square_braces)) % pairing_color_size;
                        break;
                    case '{':
                        pairing_color_index = static_cast<std::size_t>(std::abs(line_color_context_counts.open_curly_braces)) % pairing_color_size;
                        line_color_context_counts.open_curly_braces++;
                        break;
                    case '}':
                        line_color_context_counts.open_curly_braces--;
                        pairing_color_index = static_cast<std::size_t>(std::abs(line_color_context_counts.open_curly_braces)) % pairing_color_size;
                        break;
                    default:
                        break;
                }

                fmt::print(ss, "{}", fmt::styled(c, fmt::fg(pairing_color[pairing_color_index])));
            }
            else
                fmt::print(ss, "{}", c);
        }
    }

    void makeContext(
        std::ostream& os,
        const std::string& filename,
        const std::optional<std::string>& expr,
        const std::size_t sym_size,
        const std::size_t target_line,
        const std::size_t col_start,
        const std::optional<CodeErrorContext>& maybe_context,  // can not be populated at runtime, only compile time
        const bool whole_line,
        const bool colorize)
    {
        assert(!(maybe_context && whole_line) && "Can not create error context when a context is given AND the whole line has to be underlined");

        using namespace Ark::literals;

        auto show_file_location = [&] {
            if (filename != ARK_NO_NAME_FILE)
                fmt::print(os, "In file {}:{}\n", filename, target_line + 1);
            if (expr)
                fmt::print(os, "At {} @ {}:{}\n", expr.value(), target_line + 1, col_start);
        };

        auto compute_start_end_window = [](const std::size_t center_of_window, const std::size_t line_count) {
            std::size_t start = center_of_window >= 3 ? center_of_window - 3 : 0;
            std::size_t end = center_of_window + 3 <= line_count ? center_of_window + 3 : line_count;
            return std::make_pair(start, end);
        };

        auto print_line = [&os, colorize](const std::size_t i, const std::vector<std::string>& lines, LineColorContextCounts& color_context) {
            // show current line with its number
            fmt::print(os, "{: >5} |{}", i + 1, !lines[i].empty() ? " " : "");
            if (colorize)
                colorizeLine(lines[i], color_context, os);
            else
                fmt::print(os, "{}", lines[i]);
            fmt::print(os, "\n");
        };

        const std::string line_no_num = "      |";

        auto print_context_hint = [&os, &maybe_context, &line_no_num, colorize]() mutable {
            if (!maybe_context)
                return;

            fmt::print(os, "{}", line_no_num);
            fmt::print(
                os,
                "{: <{}}{}\n",
                // padding os spaces
                " ",
                std::max(1_z, maybe_context->col),  // fixing padding when the error is on the first character
                // underline the parent of the error in red
                fmt::styled(
                    maybe_context->is_macro_expansion ? "^ macro expansion started here" : "^ expression started here",
                    colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
        };

        const std::string code = filename == ARK_NO_NAME_FILE ? "" : Utils::readFile(filename);
        const std::vector<std::string> lines = Utils::splitString(code, '\n');
        if (target_line >= lines.size() || code.empty())
        {
            // show the "in file..." before early return
            show_file_location();
            return;
        }

        auto [first_line, last_line] = compute_start_end_window(target_line, lines.size());
        // overflow is non-zero when the expression doesn't fit on the target line
        std::size_t overflow = (col_start + sym_size <= lines[target_line].size()) ? 0 : sym_size;

        const bool ctx_same_file = maybe_context && maybe_context->filename == filename;
        const bool ctx_in_window = ctx_same_file && maybe_context &&
            maybe_context->line >= first_line &&
            maybe_context->line < last_line;

        std::size_t start_line_skipping_at = 0;
        std::size_t stop_line_skipping_at = first_line;
        if (ctx_same_file && !ctx_in_window)
        {
            // showing the context will require an ellipsis, to avoid showing too many lines in the error message
            if (maybe_context->line + 3 < first_line)
                start_line_skipping_at = maybe_context->line + 3;
            else
                stop_line_skipping_at = start_line_skipping_at;

            // due to how context works, if it points to the same file,
            // we are guaranteed it will be before our error
            first_line = maybe_context->line >= 3 ? maybe_context->line - 3 : 0;
        }
        else if (maybe_context && !ctx_same_file && !maybe_context->filename.empty())
        {
            // show the location of the parent of our error first
            fmt::print(os, "Error originated from file {}:{}\n", maybe_context->filename, maybe_context->line + 1);

            const std::vector<std::string> ctx_source_lines = Utils::splitString(Utils::readFile(maybe_context->filename), '\n');
            auto [ctx_first_line, ctx_last_line] = compute_start_end_window(maybe_context->line, ctx_source_lines.size());
            LineColorContextCounts line_color_context_counts;

            for (auto i = ctx_first_line; i < ctx_last_line; ++i)
            {
                print_line(i, ctx_source_lines, line_color_context_counts);
                if (i == maybe_context->line)
                    print_context_hint();
            }

            fmt::print(os, "\n");
        }

        show_file_location();
        LineColorContextCounts line_color_context_counts;

        for (auto i = first_line; i < last_line && i < lines.size(); ++i)
        {
            if (i >= start_line_skipping_at && i < stop_line_skipping_at)
                continue;
            print_line(i, lines, line_color_context_counts);

            // if the error context is in the current file, point to it as the parent of our error
            if (maybe_context && i == maybe_context->line && i != target_line)
                print_context_hint();

            // if the next line number wants us to skip line, and start != stop (meaning they got adjusted),
            // display an ellipsis
            if (i + 1 == start_line_skipping_at && i + 1 != stop_line_skipping_at)
                fmt::print(os, "  ... |\n");

            // show where the error occurred (do not mark empty lines as being part of the error when we have overflow)
            if (i == target_line || (i > target_line && overflow > 0 && !lines[i].empty()))
            {
                fmt::print(os, "{}", line_no_num);

                if (!whole_line)
                {
                    std::size_t line_first_char = lines[i].find_first_not_of(" \t\v");
                    line_first_char = line_first_char == std::string::npos ? 0 : line_first_char;

                    // if we have an overflow then we start at the beginning of the line (first non-space character)
                    const std::size_t curr_col_start = (i == target_line) ? col_start : (overflow == 0 ? col_start : line_first_char + 1);
                    // if we have an overflow, it is used as the end of the line
                    const std::size_t col_end = (i == target_line) ? std::min<std::size_t>(col_start + sym_size, lines[target_line].size())
                                                                   : std::min<std::size_t>(line_first_char + overflow, lines[i].size());
                    // update the overflow to avoid going here again if not needed
                    // using min between overflow and what we need to delete to avoid underflow
                    overflow -= std::min(overflow, lines[i].size() - line_first_char);
                    // if there is overflow left, and it's the last line of the context, extend it
                    if (overflow > 0 && i + 1 == last_line)
                        ++last_line;

                    // show the error where it's at, using the normal process, if there is no context OR if the context line is different from the error line
                    if (!maybe_context || maybe_context->line != target_line)
                        fmt::print(
                            os,
                            "{: <{}}{:~<{}}\n",
                            // padding of spaces
                            " ",
                            std::max(1_z, std::min(curr_col_start, col_end)),  // fixing padding when the error is on the first character
                            // underline the error in red
                            fmt::styled("^", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                            curr_col_start < col_end ? col_end - curr_col_start : 1);
                    else if (i == target_line)  // maybe_context has a value, i == target_line to avoid having to deal with overflow
                    {
                        const auto padding_size = std::max(1_z, maybe_context->col);

                        fmt::print(
                            os,
                            "{: <{}}{}{}{}\n",
                            // padding of spaces
                            " ",
                            padding_size,
                            // indicate where the parent is, with color
                            fmt::styled("│", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                            // yet another padding of spaces between the parent and error column (if need be)
                            // -2 to account for the │ and then └
                            (col_start - maybe_context->col <= 2) ? "" : fmt::format("{: <{}}", " ", col_start - maybe_context->col - 2),
                            // underline the error in red
                            fmt::styled("└─ error", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
                        // new line, some spacing between the error and the parent
                        fmt::print(os, "{}{: <{}}{}\n", line_no_num, " ", padding_size, fmt::styled("│", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
                        // new line, now show the "expression started here for the source"
                        fmt::print(
                            os,
                            "{}{: <{}}{}\n",
                            line_no_num,
                            // padding of spaces
                            " ",
                            padding_size,
                            fmt::styled(
                                maybe_context->is_macro_expansion ? "└─ macro expansion started here" : "└─ expression started here",
                                colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
                    }
                }
                else
                {
                    // first non-whitespace character of the line
                    // +1 for the leading whitespace after `    |` before the code
                    const std::size_t curr_col_start = lines[i].find_first_not_of(" \t\v") + 1;

                    // highlight the current line but skip any leading whitespace
                    fmt::print(
                        os,
                        "{: <{}}{:~<{}}\n",
                        // padding of spaces
                        " ",
                        curr_col_start,
                        // underline the whole line in red
                        fmt::styled("^", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                        lines[target_line].size() - curr_col_start);
                }
            }
        }
    }

    void helper(std::ostream& os, const std::string& message, const bool colorize,
                const std::string& filename,
                const std::optional<std::string>& expr, const std::size_t sym_size,
                const std::size_t line, const std::size_t column,
                const std::optional<CodeErrorContext>& maybe_context = std::nullopt)
    {
        makeContext(os, filename, expr, sym_size, line, column, maybe_context, /* whole_line= */ false, colorize);

        const auto message_lines = Utils::splitString(message, '\n');
        for (const auto& text : message_lines)
            fmt::print(os, "        {}\n", text);
    }

    std::string makeContextWithNode(const std::string& message, const internal::Node& node)
    {
        std::stringstream ss;

        std::size_t size = 3;
        if (node.isStringLike())
            size = node.string().size();

        helper(
            ss,
            message,
            true,
            node.filename(),
            node.repr(),
            size,
            node.line(),
            node.col());

        return ss.str();
    }

    void generate(const CodeError& e, std::ostream& os, bool colorize)
    {
#ifdef ARK_BUILD_EXE
        if (const char* nocolor = std::getenv("NOCOLOR"); nocolor != nullptr)
            colorize = false;
#endif

        std::string escaped_symbol;
        if (e.context.symbol.has_value())
        {
            switch (e.context.symbol.value().codepoint())
            {
                case '\n': escaped_symbol = "'\\n'"; break;
                case '\r': escaped_symbol = "'\\r'"; break;
                case '\t': escaped_symbol = "'\\t'"; break;
                case '\v': escaped_symbol = "'\\v'"; break;
                case '\0': escaped_symbol = "EOF"; break;
                case ' ': escaped_symbol = "' '"; break;
                default:
                    escaped_symbol = e.context.symbol.value().c_str();
            }
        }
        else
            escaped_symbol = e.context.expr;

        helper(
            os,
            e.what(),
            colorize,
            e.context.filename,
            escaped_symbol,
            e.context.expr.size(),
            e.context.line,
            e.context.col,
            e.additional_context);
    }
}
