// return a code page between the finalized ones and the temporary ones
// based on index sent
inline std::vector<uint8_t>& Compiler::page(int i) noexcept
{
    if (i >= 0)
        return m_code_pages[i];
    return m_temp_pages[-i - 1];
}

inline std::vector<uint8_t>* Compiler::page_ptr(int i) noexcept
{
    if (i >= 0)
        return &m_code_pages[i];
    return &m_temp_pages[-i - 1];
}

inline std::size_t Compiler::countArkObjects(const std::vector<internal::Node>& lst) noexcept
{
    std::size_t n = 0;
    for (const internal::Node& node : lst)
    {
        if (node.nodeType() != internal::NodeType::GetField)
            n++;
    }
    return n;
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
                           [&name](const std::pair<std::string, Value>& element) -> bool {
                               return name == element.first;
                           });
    if (it != internal::Builtins::builtins.end())
        return std::distance(internal::Builtins::builtins.begin(), it);
    return {};
}

inline std::optional<internal::Instruction> Compiler::isSpecific(const std::string& name) noexcept
{
    if (name == "list")
        return internal::Instruction::LIST;
    else if (name == "append")
        return internal::Instruction::APPEND;
    else if (name == "concat")
        return internal::Instruction::CONCAT;
    else if (name == "append!")
        return internal::Instruction::APPEND_IN_PLACE;
    else if (name == "concat!")
        return internal::Instruction::CONCAT_IN_PLACE;
    else if (name == "pop")
        return internal::Instruction::POP_LIST;
    else if (name == "pop!")
        return internal::Instruction::POP_LIST_IN_PLACE;

    return {};
}

inline void Compiler::pushSpecificInstArgc(internal::Instruction inst, uint16_t previous, int p) noexcept
{
    if (inst == internal::Instruction::LIST)
        pushNumber(previous, &page(p));
    else if (inst == internal::Instruction::APPEND || inst == internal::Instruction::APPEND_IN_PLACE ||
             inst == internal::Instruction::CONCAT || inst == internal::Instruction::CONCAT_IN_PLACE)
        pushNumber(previous - 1, &page(p));
}

inline bool Compiler::mayBeFromPlugin(const std::string& name) noexcept
{
    std::string splitted = Utils::splitString(name, ':')[0];
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                           [&splitted](const std::string& plugin) -> bool {
                               return std::filesystem::path(plugin).stem().string() == splitted;
                           });
    return it != m_plugins.end();
}
