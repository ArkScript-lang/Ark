inline void Optimizer::throwOptimizerError(const std::string& message, const internal::Node& node)
{
    std::stringstream ss;
    ss << message << "\n";
    if (node.filename() != ARK_NO_NAME_FILE)
        ss << "In file " << node.filename() << "\n";
    ss << "On line " << (node.line() + 1) << ":" << node.col() << ", got NodeType::";
    ss << internal::typeToString(node.nodeType()) << "\n";

    if (node.filename() != ARK_NO_NAME_FILE)
    {
        std::vector<std::string> ctx = Utils::splitString(Utils::readFile(node.filename()), '\n');

        for (int i=3; i > -3; --i)
        {
            int iline = static_cast<int>(node.line());
            if (iline >= i)
                // + 1 to display real lines numbers
                ss << std::setw(5) << (iline - i + 1) << " | " << ctx[iline - i] << "\n";
            if (i == 0)  // line of the error
            {
                ss << "      | ";
                // padding of spaces
                for (std::size_t j=0; (node.string().size() > node.col()) ? false : (j < node.col()); ++j)
                    ss << " ";
                // show the error
                for (std::size_t j=0; (node.string().size() > node.col()) ? (j < ctx[node.line()].size()) : (j < node.string().size()); ++j)
                    ss << "^";
                ss << "\n";
            }
        }
    }

    throw Ark::OptimizerError(ss.str());
}