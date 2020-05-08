inline void UserType::setOStream(FuncStream_t&& f)
{
    m_ostream_func = std::move(f);
}

inline void* UserType::data() const
{
    return m_data;
}

// custom operators
inline bool UserType::not_() const
{
    // TODO let the user implement his/her own
    return false;
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
    if (A.m_ostream_func != nullptr)
        return A.m_ostream_func(os, A);

    os << "UserType<" << A.m_type_id << ", 0x" << A.m_data << ">";
    return os;
}