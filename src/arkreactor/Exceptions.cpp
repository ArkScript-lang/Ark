#include <Ark/Exceptions.hpp>

#include <termcolor/proxy.hpp>
#include <sstream>

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

    std::string colorizeLine(const std::string& line, LineColorContextCounts& line_color_context_counts)
    {
        constexpr std::array<std::ostream& (*)(std::ostream & stream), 3> pairing_color {
            termcolor::bright_blue,
            termcolor::bright_green,
            termcolor::bright_yellow
        };
        std::size_t pairing_color_size = pairing_color.size();

        std::stringstream colorized_line;
        colorized_line << termcolor::colorize;

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
                }

                colorized_line << pairing_color[pairing_color_index] << c << termcolor::reset;
            }
            else
                colorized_line << c;
        }

        return colorized_line.str();
    }

    void makeContext(std::ostream& os, const std::string& code, std::size_t line, std::size_t col_start, std::size_t sym_size)
    {
        os << termcolor::colorize;
        std::vector<std::string> ctx = Utils::splitString(code, '\n');

        std::size_t first = line >= 3 ? line - 3 : 0;
        std::size_t last = (line + 3) <= ctx.size() ? line + 3 : ctx.size();
        std::size_t overflow = (col_start + sym_size < ctx[line].size()) ? 0 : col_start + sym_size - ctx[line].size();  // number of characters that are on more lines below
        LineColorContextCounts line_color_context_counts;

        for (std::size_t loop = first; loop < last; ++loop)
        {
            std::string current_line = colorizeLine(ctx[loop], line_color_context_counts);
            os << termcolor::green << std::setw(5) << (loop + 1) << termcolor::reset
               << " | " << current_line << "\n";

            if (loop == line || (loop > line && overflow > 0))
            {
                os << "      | ";
                // if we have an overflow then we start at the beginning of the line
                std::size_t curr_col_start = (overflow == 0) ? col_start : 0;
                // if we have an overflow, it is used as the end of the line
                std::size_t col_end = (loop == line) ? std::min<std::size_t>(col_start + sym_size, ctx[line].size())
                                                     : std::min<std::size_t>(overflow, ctx[loop].size());
                // update the overflow to avoid going here again if not needed
                overflow = (overflow > ctx[loop].size()) ? overflow - ctx[loop].size() : 0;

                // padding of spaces
                for (std::size_t i = 0; i < curr_col_start; ++i)
                    os << " ";

                // underline the error
                os << termcolor::red << "^";
                for (std::size_t i = curr_col_start + 1; i <= col_end; ++i)
                    os << "~";

                os << termcolor::reset << "\n";
            }
        }
    }

    template <typename T>
    void helper(std::ostream& os, const std::string& message, const std::string& filename, const std::string& code, const T& expr, std::size_t line, std::size_t column, std::size_t sym_size)
    {
        if (filename != ARK_NO_NAME_FILE)
            os << "In file " << filename << "\n";
        os << "At " << expr << " @ " << line << ":" << column << "\n";

        if (!code.empty())
            makeContext(os, std::move(code), line, column, sym_size);
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
            node,
            node.line(),
            node.col(),
            size);

        return ss.str();
    }

    void generate(const CodeError& e, std::string code)
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
            std::cout,
            e.what(),
            e.filename,
            file_content,
            escaped_symbol,
            e.line + 1,
            e.col + 1,
            e.expr.size());
    }
}
