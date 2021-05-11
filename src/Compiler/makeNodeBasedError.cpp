#include <Ark/Compiler/makeNodeBasedError.hpp>

#include <vector>
#include <iomanip>
#include <Ark/Constants.hpp>
#include <Ark/Utils.hpp>

namespace Ark::internal
{
    std::string makeNodeBasedErrorCtx(const std::string& message, const Ark::internal::Node& node)
    {
        std::stringstream ss;
        ss << message << "\n";
        if (node.filename() != ARK_NO_NAME_FILE)
            ss << "In file " << node.filename() << "\n";
        ss << "On line " << (node.line() + 1) << ":" << node.col() << ", got `" << node << "'";
        ss << " (NodeType::" << Ark::internal::typeToString(node) << "\n";

        std::size_t ssize = 1;
        if (node.nodeType() == internal::NodeType::Symbol || node.nodeType() == internal::NodeType::String
            || node.nodeType() == internal::NodeType::Spread)
            ssize = node.string().size();

        if (node.filename() != ARK_NO_NAME_FILE)
        {
            std::vector<std::string> ctx = Ark::Utils::splitString(Ark::Utils::readFile(node.filename()), '\n');

            for (int i=3; i > -3; --i)
            {
                int iline = static_cast<int>(node.line());
                if (iline - i >= 0 && iline - i < ctx.size())
                    // + 1 to display real lines numbers
                    ss << std::setw(5) << (iline - i + 1) << " | " << ctx[iline - i] << "\n";
                if (i == 0)  // line of the error
                {
                    ss << "      | ";
                    // padding of spaces
                    for (std::size_t j=0; (ssize > node.col()) ? false : (j < node.col()); ++j)
                        ss << " ";
                    // show the error
                    for (std::size_t j=0; (ssize > node.col()) ? (j < ctx[node.line()].size()) : (j < ssize); ++j)
                        ss << "^";
                    ss << "\n";
                }
            }
        }

        return ss.str();
    }
}