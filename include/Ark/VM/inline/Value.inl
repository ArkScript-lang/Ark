// public getters

inline ValueType Value::valueType() const noexcept
{
    // the type is stored on the right most bits
    return static_cast<ValueType>(m_const_type & (0b01111111));
}

inline bool Value::isFunction() const noexcept  // if it's a function we can resolve it
{
    auto type = valueType();
    return type == ValueType::PageAddr || type == ValueType::Closure || type == ValueType::CProc ||
            (type == ValueType::Reference && reference()->isFunction());
}

inline double Value::number() const
{
    return variant_get<double>(m_value);
}

inline const String& Value::string() const
{
    return variant_get<String>(m_value);
}

inline const std::vector<Value>& Value::constList() const
{
    return variant_get<std::vector<Value>>(m_value);
}

inline const UserType& Value::usertype() const
{
    return variant_get<UserType>(m_value);
}

// private getters

inline internal::PageAddr_t Value::pageAddr() const
{
    return variant_get<internal::PageAddr_t>(m_value);
}

inline const Value::ProcType& Value::proc() const
{
    return variant_get<Value::ProcType>(m_value);
}

inline const internal::Closure& Value::closure() const
{
    return variant_get<internal::Closure>(m_value);
}

inline const bool Value::isConst() const noexcept
{
    return m_const_type & (1 << 7);
}

inline void Value::setConst(bool value) noexcept
{
    if (value)
        m_const_type |= 1 << 7;
    else
        m_const_type &= 0b01111111;  // keep only the right most bits
}

// operators

inline bool operator==(const Value& A, const Value& B) noexcept
{
    // values should have the same type
    if (A.valueType() != B.valueType())
        return false;
    // all the types >= Nil are Nil itself, True, False, Undefined
    else if ((A.m_const_type & 0b01111111) >= static_cast<int>(ValueType::Nil))
        return true;

    return A.m_value == B.m_value;
}

inline bool operator<(const Value& A, const Value& B) noexcept
{
    if (A.valueType() != B.valueType())
        return (static_cast<int>(A.valueType()) - static_cast<int>(B.valueType())) < 0;
    return A.m_value < B.m_value;
}

inline bool operator!=(const Value& A, const Value& B) noexcept
{
    return !(A == B);
}

inline bool operator!(const Value& A) noexcept
{
    switch (A.valueType())
    {
        case ValueType::List:
            return A.constList().empty();

        case ValueType::Number:
            return !A.number();

        case ValueType::String:
            return A.string().size() == 0;

        case ValueType::User:
        case ValueType::Nil:
        case ValueType::False:
            return true;

        case ValueType::True:
            return false;

        default:
            return false;
    }
}