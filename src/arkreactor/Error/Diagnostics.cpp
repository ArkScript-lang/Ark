#include <Ark/Error/Diagnostics.hpp>

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
    using namespace Ark::literals;

    void showFileLocation(std::ostream& os, const ErrorLocation& loc)
    {
        if (loc.filename != ARK_NO_NAME_FILE)
            fmt::print(os, "In file {}:{}\n", loc.filename, loc.start.line + 1);
    }

    void hintWithContext(std::ostream& os, const std::optional<CodeErrorContext>& maybe_context, const bool colorize)
    {
        if (!maybe_context)
            return;

        fmt::print(os, "{}", Printer::GhostLinePrefix);
        fmt::print(
            os,
            "{: <{}}{}\n",
            // padding os spaces
            " ",
            std::max(1_z, maybe_context->at.start.column),  // fixing padding when the error is on the first character
            // underline the parent of the error in red
            fmt::styled(
                maybe_context->is_macro_expansion ? "^ macro expansion started here" : "^ expression started here",
                colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
    }

    void makeContext(
        const ErrorLocation& loc,
        std::ostream& os,
        const std::optional<CodeErrorContext>& maybe_context,  // can not be populated at runtime, only compile time
        const bool colorize)
    {
        assert(!(maybe_context && loc.wholeLineIsError()) && "Can not create error context when a context is given AND the whole line has to be underlined");

        Printer source_printer(loc.filename, loc.start.line, loc.maybeEndLine(), colorize);
        if (!source_printer.hasContent())
        {
            showFileLocation(os, loc);
            return;
        }

        const bool ctx_same_file = maybe_context && maybe_context->filename == loc.filename;
        const bool ctx_in_window = ctx_same_file && maybe_context && source_printer.coversLine(maybe_context->at.start.line);

        if (ctx_same_file && !ctx_in_window)
            source_printer.extendWindow(maybe_context->at.start.line);
        else if (maybe_context && !ctx_same_file && !maybe_context->filename.empty())
        {
            // show the location of the parent of our error first
            fmt::print(os, "Error originated from file {}:{}\n", maybe_context->filename, maybe_context->at.start.line + 1);

            std::optional<decltype(internal::FilePos::line)> maybe_end_line = std::nullopt;
            if (maybe_context->at.end)
                maybe_end_line = maybe_context->at.end->line;
            Printer printer(maybe_context->filename, maybe_context->at.start.line, maybe_end_line, colorize);

            while (printer.hasContent())
            {
                printer.printLine(os);
                if (printer.isTargetLine())
                    hintWithContext(os, maybe_context, colorize);
            }

            fmt::print(os, "\n");
        }

        showFileLocation(os, loc);

        while (source_printer.hasContent())
        {
            const std::size_t i = source_printer.current();
            const std::string& line = source_printer.currentLine();

            source_printer.printLine(os);

            // if the error context is in the current file, point to it as the parent of our error
            if (ctx_same_file && i == maybe_context->at.start.line && i != loc.start.line)
                hintWithContext(os, maybe_context, colorize);

            // show where the error occurred
            if (source_printer.isTargetLine() && !line.empty())
            {
                fmt::print(os, "{}", Printer::GhostLinePrefix);

                if (!loc.wholeLineIsError())
                {
                    const std::size_t line_first_char = (line.find_first_not_of(" \t\v") == std::string::npos) ? 0 : line.find_first_not_of(" \t\v");

                    const std::size_t col_start = (i == loc.start.line) ? loc.start.column : line_first_char + 1;
                    // due to the `!loc.wholeLineIsError()` check, we are guaranteed to have a value in loc.end
                    const std::size_t col_end = (i == loc.end->line) ? loc.end->column : line.size();

                    // show the error where it's at, using the normal process, if there is no context OR
                    // if the context line is different from the error line
                    if (!maybe_context || maybe_context->at.start.line != loc.start.line)
                        fmt::print(
                            os,
                            "{: <{}}{:~<{}}\n",
                            // padding of spaces
                            " ",
                            std::max(1_z, std::min(col_start, col_end)),  // fixing padding when the error is on the first character
                            // underline the error in red
                            fmt::styled("^", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                            col_start < col_end ? col_end - col_start : 1);
                    // cppcheck-suppress knownConditionTrueFalse ; suppressing error so that the condition is explicit to the reader
                    else if (maybe_context && maybe_context->at.start.line == loc.start.line && i == loc.start.line)
                    {
                        const auto padding_size = std::max(1_z, maybe_context->at.start.column);
                        const std::string inner_padding =
                            // -2 to account for the │ and then └
                            (loc.start.column < padding_size || loc.start.column - padding_size < 2)
                            ? ""
                            : std::string(std::max(1_z, loc.start.column - padding_size - 1), ' ');

                        fmt::print(
                            os,
                            "{: <{}}{}{}{}\n",
                            // padding of spaces
                            " ",
                            padding_size,
                            // indicate where the parent is, with color
                            fmt::styled("│", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                            // yet another padding of spaces between the parent and error column (if need be)
                            inner_padding,
                            // underline the error in red
                            fmt::styled("└─ error", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
                        // new line, some spacing between the error and the parent
                        fmt::print(
                            os,
                            "{}{: <{}}{}\n", Printer::GhostLinePrefix,
                            " ",
                            padding_size,
                            fmt::styled("│", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()));
                        // new line, now show the "expression started here for the source"
                        fmt::print(
                            os,
                            "{}{: <{}}{}\n",
                            Printer::GhostLinePrefix,
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
                    const std::size_t col_start = line.find_first_not_of(" \t\v") + 1;

                    // highlight the current line but skip any leading whitespace
                    fmt::print(
                        os,
                        "{: <{}}{:~<{}}\n",
                        // padding of spaces
                        " ",
                        col_start,
                        // underline the whole line in red
                        fmt::styled("^", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                        line.size() - col_start);
                }
            }
        }
    }

    void helper(std::ostream& os, const std::string& message, const bool colorize,
                const std::string& filename, const internal::FileSpan& at,
                const std::optional<CodeErrorContext>& maybe_context = std::nullopt)
    {
        makeContext(
            ErrorLocation {
                .filename = filename,
                .start = at.start,
                .end = at.end },
            os, maybe_context, colorize);

        for (const auto& text : Utils::splitString(message, '\n'))
            fmt::print(os, "        {}\n", text);
    }

    std::string makeContextWithNode(const std::string& message, const internal::Node& node)
    {
        std::stringstream ss;

        helper(
            ss,
            message,
            true,
            node.filename(),
            node.position());

        return ss.str();
    }

    void generate(const CodeError& e, std::ostream& os, bool colorize)
    {
#ifdef ARK_BUILD_EXE
        if (const char* nocolor = std::getenv("NOCOLOR"); nocolor != nullptr)
            colorize = false;
#endif

        helper(
            os,
            e.what(),
            colorize,
            e.context.filename,
            e.context.at,
            e.additional_context);
    }
}
