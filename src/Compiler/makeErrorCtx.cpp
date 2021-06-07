#include <Ark/Compiler/makeErrorCtx.hpp>

#include <vector>
#include <iomanip>

#include <Ark/Constants.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    void makeContext(std::ostream& os, const std::string& code, std::size_t line, std::size_t col_start, std::size_t col_end)
    {
        std::vector<std::string> ctx = Utils::splitString(code, '\n');
        int iline = static_cast<int>(line);

        for (int i = 3; i > -3; --i)
        {
            if (iline - i >= 0 && iline - i < ctx.size())
                // + 1 to display real lines numbers
                os << std::setw(5) << (iline - i + 1) << " | " << ctx[iline - i] << "\n";

            if (i == 0)  // line of the error
            {
                os << "      | ";
                // padding of spaces
                for (std::size_t j = 0; (ssize > col_start) ? false : (j < col_start); ++j)
                    os << " ";
                // show the error
                for (std::size_t j = 0; (ssize > col_start) ? (j < col_end) : (j < ssize); ++j)
                    os << "^";
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
        if (node.nodeType() == NodeType::Symbol || node.nodeType() == NodeType::String
            || node.nodeType() == NodeType::Spread)
            ssize = node.string().size();

        if (node.filename() != ARK_NO_NAME_FILE)
            makeContext(ss, Utils::readFile(node.filename()), node.line(), node.col(), ctx[node.line()].size());

        return ss.str();
    }

    std::string makeTokenBasedErrorCtx(const std::string& match, std::size_t line, std::size_t col, const std::string& code)
    {
        std::stringstream ss;
        ss << message << "\n";
        if (??? != ARK_NO_NAME_FILE)
            ss << "In file " << ??? << "\n";
        ss << "On line " << (line + 1) << ":" << col << ", got TokenType::";
        ss << tokentype_string[static_cast<unsigned>(token.type)] << "\n";

        makeContext(ss, code, line, col, match.size());

        return ss.str();
    }
}