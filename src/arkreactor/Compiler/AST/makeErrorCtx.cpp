#include <Ark/Compiler/AST/makeErrorCtx.hpp>

#include <vector>
#include <iomanip>
#include <termcolor/proxy.hpp>

#include <Ark/Constants.hpp>
#include <Ark/Files.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    void makeContext(std::ostream& os, const std::string& code, std::size_t line, std::size_t col_start, std::size_t sym_size)
    {
        os << termcolor::colorize;
        std::vector<std::string> ctx = Utils::splitString(code, '\n');

        std::size_t col_end = std::min<std::size_t>(col_start + sym_size, ctx[line].size());
        std::size_t first = line >= 3 ? line - 3 : 0;
        std::size_t last = (line + 3) <= ctx.size() ? line + 3 : ctx.size();
        LineColorContextCounts line_color_context_counts;

        for (std::size_t loop = first; loop < last; ++loop)
        {
            std::string current_line = colorizeLine(ctx[loop], line_color_context_counts);
            os << termcolor::green << std::setw(5) << (loop + 1) << termcolor::reset << " | " << current_line << "\n";

            if (loop == line)
            {
                os << "      | ";

                // padding of spaces
                for (std::size_t i = 0; i < col_start; ++i)
                    os << " ";

                // underline the error
                os << termcolor::red;
                for (std::size_t i = col_start; i < col_end; ++i)
                    os << "^";

                os << termcolor::reset << "\n";
            }
        }
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

    std::string makeNodeBasedErrorCtx(const std::string& message, const Node& node)
    {
        std::stringstream ss;
        ss << message << "\n\n";
        if (node.filename() != ARK_NO_NAME_FILE)
            ss << "In file " << node.filename() << "\n";
        ss << "On line " << (node.line() + 1) << ":" << node.col() << ", got `" << node << "'\n";

        std::size_t ssize = 1;
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::String || node.nodeType() == NodeType::Spread)
            ssize = node.string().size();

        if (node.filename() != ARK_NO_NAME_FILE)
            makeContext(ss, Utils::readFile(node.filename()), node.line(), node.col(), ssize);

        return ss.str();
    }

    std::string makeTokenBasedErrorCtx(const std::string& match, std::size_t line, std::size_t col, const std::string& code)
    {
        std::stringstream ss;
        ss << "On line " << (line + 1) << ":" << col << "\n";
        makeContext(ss, code, line, col, match.size());

        return ss.str();
    }
}
