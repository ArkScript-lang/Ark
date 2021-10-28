#include <Ark/Compiler/AST/makeErrorCtx.hpp>

#include <vector>
#include <iomanip>

#include <Ark/Constants.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    void makeContext(std::ostream& os, const std::string& code, std::size_t line, std::size_t col_start, std::size_t sym_size)
    {
        std::vector<std::string> ctx = Utils::splitString(code, '\n');

        std::size_t col_end = std::min(col_start + sym_size, ctx[line].size());
        std::size_t first = line >= 3 ? line - 3 : 0;
        std::size_t last = (line + 3) <= ctx.size() ? line + 3 : ctx.size();

        for (std::size_t loop = first; loop < last; ++loop)
        {
            os << std::setw(5) << (loop + 1) << " | " << ctx[loop] << "\n";

            if (loop == line)
            {
                os << "      | ";

                for (std::size_t j = 0; (sym_size > col_start) ? false : (j < col_start); ++j)
                {
                    if (j < col_start)  // padding of spaces
                        os << " ";
                    else if (j >= col_start && j <= col_end)  // underline the error
                        os << "^";
                }
                os << "\n";
            }
        }
    }

    std::string makeNodeBasedErrorCtx(const std::string& message, const Node& node)
    {
        std::stringstream ss;
        ss << message << "\n";
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
