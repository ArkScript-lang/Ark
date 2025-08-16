#include <Ark/Error/Exceptions.hpp>

#include <cassert>
#include <sstream>
#include <algorithm>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <Ark/Constants.hpp>
#include <Ark/Utils/Utils.hpp>
#include <Ark/Utils/Literals.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Error/PrettyPrinting.hpp>

namespace Ark::Diagnostics
{
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

        Printer source_printer(filename, target_line, colorize);
        if (!source_printer.hasContent())
        {
            // show the "in file..." before early return
            show_file_location();
            return;
        }
        const std::string& targetLine = source_printer.targetLine();

        // fixme: migrate to end line/end column
        // overflow is non-zero when the expression doesn't fit on the target line
        std::size_t overflow = (col_start + sym_size <= targetLine.size()) ? 0 : sym_size;

        const bool ctx_same_file = maybe_context && maybe_context->filename == filename;
        const bool ctx_in_window = ctx_same_file && maybe_context && source_printer.coversLine(maybe_context->line);

        if (ctx_same_file && !ctx_in_window)
            source_printer.extendWindow(maybe_context->line);
        else if (maybe_context && !ctx_same_file && !maybe_context->filename.empty())
        {
            // show the location of the parent of our error first
            fmt::print(os, "Error originated from file {}:{}\n", maybe_context->filename, maybe_context->line + 1);

            Printer printer(maybe_context->filename, maybe_context->line, colorize);
            while (printer.hasContent())
            {
                printer.printLine(os);
                if (printer.isTargetLine())
                    print_context_hint();  // todo: migrate to pretty printer?
            }

            fmt::print(os, "\n");
        }

        show_file_location();

        while (source_printer.hasContent())
        {
            const std::size_t i = source_printer.current();
            const std::string& line = source_printer.currentLine();

            source_printer.printLine(os);

            // if the error context is in the current file, point to it as the parent of our error
            if (maybe_context && i == maybe_context->line && i != target_line)
                print_context_hint();

            // show where the error occurred (do not mark empty lines as being part of the error when we have overflow)
            if (source_printer.isTargetLine() || (i > target_line && overflow > 0 && !line.empty()))
            {
                fmt::print(os, "{}", line_no_num);

                if (!whole_line)
                {
                    std::size_t line_first_char = line.find_first_not_of(" \t\v");
                    line_first_char = line_first_char == std::string::npos ? 0 : line_first_char;

                    // if we have an overflow then we start at the beginning of the line (first non-space character)
                    const std::size_t curr_col_start = (i == target_line) ? col_start : (overflow == 0 ? col_start : line_first_char + 1);
                    // if we have an overflow, it is used as the end of the line
                    const std::size_t col_end = (i == target_line) ? std::min<std::size_t>(col_start + sym_size, targetLine.size())
                                                                   : std::min<std::size_t>(line_first_char + overflow, line.size());
                    // update the overflow to avoid going here again if not needed
                    // using min between overflow and what we need to delete to avoid underflow
                    overflow -= std::min(overflow, line.size() - line_first_char);
                    // if there is overflow left, and it's the last line of the context, extend it
                    if (overflow > 0 && i + 1 == source_printer.window().end)
                        source_printer.extendWindowEnd();

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
                    const std::size_t curr_col_start = line.find_first_not_of(" \t\v") + 1;

                    // highlight the current line but skip any leading whitespace
                    fmt::print(
                        os,
                        "{: <{}}{:~<{}}\n",
                        // padding of spaces
                        " ",
                        curr_col_start,
                        // underline the whole line in red
                        fmt::styled("^", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                        targetLine.size() - curr_col_start);
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
