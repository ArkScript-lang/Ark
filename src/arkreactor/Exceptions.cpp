#include <Ark/Exceptions.hpp>

#include <sstream>
#include <algorithm>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

#include <Ark/Constants.hpp>
#include <Ark/Utils.hpp>
#include <Ark/Files.hpp>
#include <Ark/Literals.hpp>
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

        for (const char& c : line)
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

    void makeContext(std::ostream& os, const std::string& code, const std::size_t target_line, const std::size_t col_start, const std::size_t sym_size, const bool colorize)
    {
        using namespace Ark::literals;

        const std::vector<std::string> ctx = Utils::splitString(code, '\n');
        if (target_line >= ctx.size())
            return;

        const std::size_t first_line = target_line >= 3 ? target_line - 3 : 0;
        const std::size_t last_line = (target_line + 3) <= ctx.size() ? target_line + 3 : ctx.size();
        std::size_t overflow = (col_start + sym_size < ctx[target_line].size()) ? 0 : col_start + sym_size - ctx[target_line].size();  // number of characters that are on more lines below
        LineColorContextCounts line_color_context_counts;

        for (auto i = first_line; i < last_line; ++i)
        {
            fmt::print(os, "{: >5} |{}", i + 1, !ctx[i].empty() ? " " : "");
            if (colorize)
                colorizeLine(ctx[i], line_color_context_counts, os);
            else
                fmt::print(os, "{}", ctx[i]);
            fmt::print(os, "\n");

            if (i == target_line || (i > target_line && overflow > 0))
            {
                fmt::print(os, "      |");
                // if we have an overflow then we start at the beginning of the line
                const std::size_t curr_col_start = (overflow == 0) ? col_start : 0;
                // if we have an overflow, it is used as the end of the line
                const std::size_t col_end = (i == target_line) ? std::min<std::size_t>(col_start + sym_size, ctx[target_line].size())
                                                               : std::min<std::size_t>(overflow, ctx[i].size());
                // update the overflow to avoid going here again if not needed
                overflow = (overflow > ctx[i].size()) ? overflow - ctx[i].size() : 0;

                fmt::print(
                    os,
                    "{: <{}}{:~<{}}\n",
                    // padding of spaces
                    " ",
                    std::max(1_z, curr_col_start),  // fixing padding when the error is on the first character
                    // underline the error in red
                    fmt::styled("^", colorize ? fmt::fg(fmt::color::red) : fmt::text_style()),
                    col_end - curr_col_start);
            }
        }
    }

    template <typename T>
    void helper(std::ostream& os, const std::string& message, bool colorize, const std::string& filename, const std::string& code, const T& expr, const std::size_t line, std::size_t column, const std::size_t sym_size)
    {
        if (filename != ARK_NO_NAME_FILE)
            fmt::print(os, "In file {}\n", filename);
        fmt::print(os, "At {} @ {}:{}\n", expr, line + 1, column);

        if (!code.empty())
            makeContext(os, code, line, column, sym_size, colorize);
        fmt::print(os, "        {}", message);
    }

    std::string makeContextWithNode(const std::string& message, const internal::Node& node)
    {
        std::stringstream ss;

        std::size_t size = 3;
        // todo add "can be string" attribute
        if (node.nodeType() == internal::NodeType::Symbol || node.nodeType() == internal::NodeType::String || node.nodeType() == internal::NodeType::Spread)
            size = node.string().size();

        helper(
            ss,
            message,
            true,
            node.filename(),
            (node.filename() == ARK_NO_NAME_FILE) ? "" : Utils::readFile(node.filename()),
            node.repr(),
            node.line(),
            node.col(),
            size);

        return ss.str();
    }

    void generate(const CodeError& e, std::ostream& os, bool colorize)
    {
        if (const char* nocolor = std::getenv("NOCOLOR"); nocolor != nullptr)
            colorize = false;

        std::string escaped_symbol;
        if (e.symbol.has_value())
        {
            switch (e.symbol.value().codepoint())
            {
                case '\n': escaped_symbol = "'\\n'"; break;
                case '\r': escaped_symbol = "'\\r'"; break;
                case '\t': escaped_symbol = "'\\t'"; break;
                case '\v': escaped_symbol = "'\\v'"; break;
                case '\0': escaped_symbol = "EOF"; break;
                case ' ': escaped_symbol = "' '"; break;
                default:
                    escaped_symbol = e.symbol.value().c_str();
            }
        }
        else
            escaped_symbol = e.expr;

        std::string file_content;
        if (e.filename != ARK_NO_NAME_FILE)
            file_content = Utils::readFile(e.filename);

        // TODO enhance the error messages
        helper(
            os,
            e.what(),
            colorize,
            e.filename,
            file_content,
            escaped_symbol,
            e.line,
            e.col,
            e.expr.size());
    }
}
