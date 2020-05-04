// public getters

inline ValueType Value::valueType() const
{
    return m_type;
}

inline bool Value::isFunction() const  // if it's a function we can resolve it
{
    return m_type == ValueType::PageAddr || m_type == ValueType::Closure || m_type == ValueType::CProc;
}

inline double Value::number() const
{
    return std::get<double>(m_value);
}

inline const String& Value::string() const
{
    return std::get<String>(m_value);
}

inline const std::vector<Value>& Value::const_list() const
{
    return std::get<std::vector<Value>>(m_value);
}

inline const UserType& Value::usertype() const
{
    return std::get<UserType>(m_value);
}

// private getters

inline PageAddr_t Value::pageAddr() const
{
    return std::get<PageAddr_t>(m_value);
}

inline const Value::ProcType& Value::proc() const
{
    return std::get<Value::ProcType>(m_value);
}

inline const Closure& Value::closure() const
{
    return std::get<Closure>(m_value);
}

// operators

inline bool operator==(const Value& A, const Value& B)
{
    // values should have the same type
    if (A.m_type != B.m_type)
        return false;
    else if (A.m_type == ValueType::Nil || A.m_type == ValueType::True || A.m_type == ValueType::False || A.m_type == ValueType::Undefined)
        return true;

    return A.m_value == B.m_value;
}

inline bool operator<(const Value& A, const Value& B)
{
    if (A.m_type != B.m_type)
        return (static_cast<int>(A.m_type) - static_cast<int>(B.m_type)) < 0;
    return A.m_value < B.m_value;
}

inline bool operator!=(const Value& A, const Value& B)
{
    return !(A == B);
}

inline bool operator!(const Value& A)
{
    switch (A.m_type)
    {
        case ValueType::List:
            return A.const_list().empty();

        case ValueType::Number:
            return !A.number();

        case ValueType::String:
            return A.string().size() == 0;

        case ValueType::User:
            return A.usertype().not_();

        case ValueType::Nil:
        case ValueType::False:
            return true;

        case ValueType::True:
            return false;

        default:
            return false;
    }
}