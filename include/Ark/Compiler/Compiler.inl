// return a code page between the finalized ones and the temporary ones
// based on index sent
inline std::vector<internal::Inst>& Compiler::page(int i) noexcept
{
    if (i >= 0)
        return m_code_pages[i];
    return m_temp_pages[-i - 1];
}

inline std::optional<std::size_t> Compiler::isOperator(const std::string& name) noexcept
{
    auto it = std::find(internal::Builtins::operators.begin(), internal::Builtins::operators.end(), name);
    if (it != internal::Builtins::operators.end())
        return std::distance(internal::Builtins::operators.begin(), it);
    return {};
}

inline std::optional<std::size_t> Compiler::isBuiltin(const std::string& name) noexcept
{
    auto it = std::find_if(internal::Builtins::builtins.begin(), internal::Builtins::builtins.end(),
        [&name](const std::pair<std::string, internal::Value>& element) -> bool {
            return name == element.first;
    });
    if (it != internal::Builtins::builtins.end())
        return std::distance(internal::Builtins::builtins.begin(), it);
    return {};
}

inline bool Compiler::mayBeFromPlugin(const std::string& name) noexcept
{
    std::string splitted = Utils::splitString(name, ':')[0];
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
        [&splitted](const std::string& plugin) -> bool {
            return Utils::splitString(plugin, '.')[0] == splitted;
    });
    return it != m_plugins.end();
}

inline void Compiler::throwCompilerError(const std::string& message, const internal::Node& node)
{
    std::stringstream ss;
    ss << message << "\n";
    if (node.filename() != ARK_NO_NAME_FILE)
        ss << "In file " << node.filename() << "\n";
    ss << "On line " << (node.line() + 1) << ":" << node.col() << "\n";

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

    throw Ark::CompilationError(ss.str());
}