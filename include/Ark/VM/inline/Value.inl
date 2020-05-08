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
    return m_value.number;
}

inline const String& Value::string() const
{
    return m_value.string;
}

inline const std::vector<Value>& Value::const_list() const
{
    return m_value.list;
}

inline const UserType& Value::usertype() const
{
    return m_value.user;
}

// private getters

inline PageAddr_t Value::pageAddr() const
{
    return m_value.page;
}

inline const Value::ProcType& Value::proc() const
{
    return m_value.proc;
}

inline const Closure& Value::closure() const
{
    return m_value.closure;
}

// operators

inline bool operator==(const Value& A, const Value& B)
{
    // values should have the same type
    if (A.m_type != B.m_type)
        return false;

    switch (A.m_type)
    {
        case ValueType::Nil:
        case ValueType::True:
        case ValueType::False:
        case ValueType::Undefined:
            return true;

        case ValueType::List:
            return A.m_value.list == B.m_value.list;

        case ValueType::Number:
            return A.m_value.number == B.m_value.number;

        case ValueType::String:
            return A.m_value.string == B.m_value.string;

        case ValueType::User:
            return A.m_value.user == B.m_value.user;

        case ValueType::PageAddr:
            return A.m_value.page == B.m_value.page;

        case ValueType::CProc:
            return A.m_value.proc == B.m_value.proc;

        case ValueType::Closure:
            return A.m_value.closure == B.m_value.closure;

        default:
            return false;
    }
}

inline bool operator<(const Value& A, const Value& B)
{
    if (A.m_type == ValueType::True && (B.m_type == ValueType::False || B.m_type == ValueType::Nil))
        return false;
    if ((A.m_type == ValueType::False || A.m_type == ValueType::Nil) && B.m_type == ValueType::True)
        return true;

    if (A.m_type != B.m_type)
        return (static_cast<int>(A.m_type) - static_cast<int>(B.m_type)) < 0;

    switch (A.m_type)
    {
        case ValueType::List:
            return A.m_value.list < B.m_value.list;

        case ValueType::Number:
            return A.m_value.number < B.m_value.number;

        case ValueType::String:
            return A.m_value.string < B.m_value.string;

        case ValueType::User:
            return A.m_value.user < B.m_value.user;

        case ValueType::PageAddr:
            return A.m_value.page < B.m_value.page;

        case ValueType::CProc:
        case ValueType::Closure:
            return false;

        default:
            return false;
    }
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