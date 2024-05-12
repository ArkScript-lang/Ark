#include <Ark/Exceptions.hpp>

#include <termcolor/proxy.hpp>
#include <sstream>
#include <Ark/Constants.hpp>

#include <Ark/Utils.hpp>
#include <Ark/Files.hpp>
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
            termcolor::bright_blue,
            termcolor::bright_green,
            termcolor::bright_yellow
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
                        pairing_color_index = std::abs(line_color_context_counts.open_parentheses) % pairing_color_size;
                        line_color_context_counts.open_parentheses++;
                        break;
                    case ')':
                        line_color_context_counts.open_parentheses--;
                        pairing_color_index = std::abs(line_color_context_counts.open_parentheses) % pairing_color_size;
                        break;
                    case '[':
                        pairing_color_index = std::abs(line_color_context_counts.open_square_braces) % pairing_color_size;
                        line_color_context_counts.open_square_braces++;
                        break;
                    case ']':
                        line_color_context_counts.open_square_braces--;
                        pairing_color_index = std::abs(line_color_context_counts.open_square_braces) % pairing_color_size;
                        break;
                    case '{':
                        pairing_color_index = std::abs(line_color_context_counts.open_curly_braces) % pairing_color_size;
                        line_color_context_counts.open_curly_braces++;
                        break;
                    case '}':
                        line_color_context_counts.open_curly_braces--;
                        pairing_color_index = std::abs(line_color_context_counts.open_curly_braces) % pairing_color_size;
                        break;
                    default:
                        break;
                }

                ss << pairing_color[pairing_color_index] << c << termcolor::reset;
            }
            else
                ss << c;
        }
    }

    void makeContext(std::ostream& os, const std::string& code, const std::size_t target_line, const std::size_t col_start, const std::size_t sym_size)
    {
        const std::vector<std::string> ctx = Utils::splitString(code, '\n');

        const std::size_t first_line = target_line >= 3 ? target_line - 3 : 0;
        const std::size_t last_line = (target_line + 3) <= ctx.size() ? target_line + 3 : ctx.size();
        std::size_t overflow = (col_start + sym_size < ctx[target_line].size()) ? 0 : col_start + sym_size - ctx[target_line].size();  // number of characters that are on more lines below
        LineColorContextCounts line_color_context_counts;

        for (auto i = first_line; i < last_line; ++i)
        {
            os << termcolor::green << std::setw(5) << (i + 1) << termcolor::reset << " | ";
            colorizeLine(ctx[i], line_color_context_counts, os);
            os << "\n";

            if (i == target_line || (i > target_line && overflow > 0))
            {
                os << "      |";
                // if we have an overflow then we start at the beginning of the line
                const std::size_t curr_col_start = (overflow == 0) ? col_start : 0;
                // if we have an overflow, it is used as the end of the line
                const std::size_t col_end = (i == target_line) ? std::min<std::size_t>(col_start + sym_size, ctx[target_line].size())
                                                               : std::min<std::size_t>(overflow, ctx[i].size());
                // update the overflow to avoid going here again if not needed
                overflow = (overflow > ctx[i].size()) ? overflow - ctx[i].size() : 0;

                // fixing padding when the error is on the first character
                if (curr_col_start == 0)
                    os << " ";

                // padding of spaces
                for (std::size_t j = 0; j < curr_col_start; ++j)
                    os << " ";

                // underline the error
                os << termcolor::red << "^";
                for (std::size_t j = curr_col_start + 1; j < col_end; ++j)
                    os << "~";

                os << termcolor::reset << "\n";
            }
        }
    }

    template <typename T>
    void helper(std::ostream& os, const std::string& message, const std::string& filename, const std::string& code, const T& expr, const std::size_t line, std::size_t column, const std::size_t sym_size)
    {
        if (filename != ARK_NO_NAME_FILE)
            os << "In file " << filename << "\n";
        os << "At " << expr << " @ " << (line + 1) << ":" << column << "\n";

        if (!code.empty())
            makeContext(os, code, line, column, sym_size);
        os << "        " << message;
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
            node.filename(),
            (node.filename() == ARK_NO_NAME_FILE) ? "" : Utils::readFile(node.filename()),
            node.repr(),
            node.line(),
            node.col(),
            size);

        return ss.str();
    }

    void generate(const CodeError& e, std::string code, std::ostream& os)
    {
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
        if (e.filename == ARK_NO_NAME_FILE)
            file_content = std::move(code);
        else
            file_content = Utils::readFile(e.filename);

        // TODO enhance the error messages
        helper(
            os,
            e.what(),
            e.filename,
            file_content,
            escaped_symbol,
            e.line,
            e.col,
            e.expr.size());
    }
}
