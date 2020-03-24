// return a code page between the finalized ones and the temporary ones
// based on index sent
inline std::vector<internal::Inst>& Compiler::page(int i)
{
    if (i >= 0)
        return m_code_pages[i];
    return m_temp_pages[-i - 1];
}

inline std::optional<std::size_t> Compiler::isOperator(const std::string& name)
{
    auto it = std::find(internal::FFI::operators.begin(), internal::FFI::operators.end(), name);
    if (it != internal::FFI::operators.end())
        return std::distance(internal::FFI::operators.begin(), it);
    return {};
}

inline std::optional<std::size_t> Compiler::isBuiltin(const std::string& name)
{
    auto it = std::find_if(internal::FFI::builtins.begin(), internal::FFI::builtins.end(),
        [&name](const std::pair<std::string, internal::Value>& element) -> bool {
            return name == element.first;
    });
    if (it != internal::FFI::builtins.end())
        return std::distance(internal::FFI::builtins.begin(), it);
    return {};
}