inline void UserType::setControlFuncs(ControlFuncs* block)
{
    m_funcs = block;
}

inline void* UserType::data() const
{
    return m_data;
}

// friends

inline bool operator==(const UserType& A, const UserType& B)
{
    return (A.m_type_id == B.m_type_id) && (A.m_data == B.m_data);
}

inline bool operator<(const UserType& A, const UserType& B)
{
    return false;
}

inline std::ostream& operator<<(std::ostream& os, const UserType& A)
{
    if (A.m_funcs != nullptr && A.m_funcs->ostream_func != nullptr)
        return A.m_funcs->ostream_func(os, A);

    os << "UserType<" << A.m_type_id << ", 0x" << A.m_data << ">";
    return os;
}