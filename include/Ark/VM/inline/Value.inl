// public getters

inline ValueType Value::valueType() const
{
    // the type is stored on the right most bits
    return static_cast<ValueType>(m_constType & (0b01111111));
}

inline bool Value::isFunction() const  // if it's a function we can resolve it
{
    auto type = valueType();
    return type == ValueType::PageAddr || type == ValueType::Closure || type == ValueType::CProc;
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

inline const bool Value::isConst() const
{
    return m_constType & (1 << 8);
}

inline void Value::setConst(bool value)
{
    if (value)
        m_constType |= 1 << 8;
    else
        m_constType &= 0b01111111;  // keep only the right most bits
}

// operators

inline bool operator==(const Value& A, const Value& B)
{
    // values should have the same type
    if (A.valueType() != B.valueType())
        return false;
    else if (A.valueType() == ValueType::Nil || A.valueType() == ValueType::True || A.valueType() == ValueType::False || A.valueType() == ValueType::Undefined)
        return true;

    return A.m_value == B.m_value;
}

inline bool operator<(const Value& A, const Value& B)
{
    if (A.valueType() != B.valueType())
        return (static_cast<int>(A.valueType()) - static_cast<int>(B.valueType())) < 0;
    return A.m_value < B.m_value;
}

inline bool operator!=(const Value& A, const Value& B)
{
    return !(A == B);
}

inline bool operator!(const Value& A)
{
    switch (A.valueType())
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