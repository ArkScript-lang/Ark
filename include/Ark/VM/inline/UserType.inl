inline void UserType::setControlFuncs(ControlFuncs* block) noexcept
{
    m_funcs = block;
}

inline void* UserType::data() const noexcept
{
    return m_data;
}