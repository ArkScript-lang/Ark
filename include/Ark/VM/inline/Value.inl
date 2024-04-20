inline bool operator==(const Value& A, const Value& B) noexcept
{
    // values should have the same type
    if (A.type_num() != B.type_num())
        return false;
    // all the types >= Nil are Nil itself, True, False, Undefined
    else if (A.type_num() >= static_cast<uint8_t>(ValueType::Nil))
        return true;

    return A.m_value == B.m_value;
}

inline bool operator<(const Value& A, const Value& B) noexcept
{
    if (A.type_num() != B.type_num())
        return (A.type_num() - B.type_num()) < 0;
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
            return A.string().empty();

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
