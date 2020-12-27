// stack related

inline Value* Frame::pop()
{
    m_i--;
    return &m_stack[m_i];
}

inline void Frame::push(const Value& value) noexcept
{
    m_stack[m_i] = value;
    m_i++;

    if (m_i == m_stack.size())
        m_stack.emplace_back(ValueType::Undefined);
}

inline void Frame::push(Value&& value) noexcept
{
    m_stack[m_i].m_value = std::move(value.m_value);
    m_stack[m_i].m_constType = std::move(value.m_constType);
    m_i++;

    if (m_i == m_stack.size())
        m_stack.emplace_back(ValueType::Undefined);
}

// getters-setters (misc)

inline std::size_t Frame::stackSize() const noexcept
{
    return m_i;
}

inline uint16_t Frame::callerAddr() const noexcept
{
    return m_addr;
}

inline uint16_t Frame::callerPageAddr() const noexcept
{
    return m_page_addr;
}