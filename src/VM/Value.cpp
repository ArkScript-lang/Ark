#include <Ark/VM/Value.hpp>

#include <Ark/Utils.hpp>

#define init_const_type(is_const, type) ((is_const ? (1 << 7) : 0) | static_cast<uint8_t>(type))

namespace Ark
{
    Value::Value() noexcept :
        m_const_type(init_const_type(false, ValueType::Undefined))
    {}

    // --------------------------

    Value::Value(ValueType type) noexcept :
        m_const_type(init_const_type(false, type))
    {
        if (type == ValueType::List)
            m_value = std::vector<Value>();
        else if (type == ValueType::String)
            m_value = "";

        #ifdef ARK_PROFILER_COUNT
            value_creations++;
        #endif
    }

#ifdef ARK_PROFILER_COUNT
    extern unsigned value_creations = 0;
    extern unsigned value_copies = 0;
    extern unsigned value_moves = 0;

    Value::Value(const Value& val) noexcept :
        m_value(val.m_value),
        m_const_type(val.m_const_type)
    {
        if (valueType() != ValueType::Reference)
            value_copies++;
    }

    Value::Value(Value&& other) noexcept
    {
        m_value = std::move(other.m_value);
        m_const_type = std::move(other.m_const_type);

        if (valueType() != ValueType::Reference)
            value_moves++;
    }

    Value& Value::operator=(const Value& other) noexcept
    {
        m_value = other.m_value;
        m_const_type = other.m_const_type;

        if (valueType() != ValueType::Reference)
            value_copies++;

        return *this;
    }
#endif

    Value::Value(int value) noexcept :
        m_value(static_cast<double>(value)), m_const_type(init_const_type(false, ValueType::Number))
    {}

    Value::Value(float value) noexcept :
        m_value(static_cast<double>(value)), m_const_type(init_const_type(false, ValueType::Number))
    {}

    Value::Value(double value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::Number))
    {}

    Value::Value(const std::string& value) noexcept :
        m_value(value.c_str()), m_const_type(init_const_type(false, ValueType::String))
    {}

    Value::Value(const String& value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::String))
    {}

    Value::Value(const char* value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::String))
    {}

    Value::Value(internal::PageAddr_t value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::PageAddr))
    {}

    Value::Value(Value::ProcType value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::CProc))
    {}

    Value::Value(std::vector<Value>&& value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::List))
    {}

    Value::Value(internal::Closure&& value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::Closure))
    {}

    Value::Value(UserType&& value) noexcept :
        m_value(value), m_const_type(init_const_type(false, ValueType::User))
    {}

    Value::Value(Value* ref) noexcept :
        m_value(ref), m_const_type(init_const_type(true, ValueType::Reference))
    {}

    // --------------------------

    std::vector<Value>& Value::list()
    {
        return variant_get<std::vector<Value>>(m_value);
    }

    internal::Closure& Value::refClosure()
    {
        return variant_get<internal::Closure>(m_value);
    }

    String& Value::stringRef()
    {
        return variant_get<String>(m_value);
    }

    UserType& Value::usertypeRef()
    {
        return variant_get<UserType>(m_value);
    }

    Value* Value::reference() const
    {
        return variant_get<Value*>(m_value);
    }

    // --------------------------

    void Value::push_back(const Value& value)
    {
        list().push_back(value);
    }

    void Value::push_back(Value&& value)
    {
        list().push_back(value);
    }

    // --------------------------

    std::ostream& operator<<(std::ostream& os, const Value& V) noexcept
    {
        switch (V.valueType())
        {
        case ValueType::Number:
        {
            double d = V.number();
            os.precision(Utils::digPlaces(d) + Utils::decPlaces(d));
            os << d;
            break;
        }

        case ValueType::String:
            os << V.string().c_str();
            break;

        case ValueType::PageAddr:
            os << "Function @ " << V.pageAddr();
            break;

        case ValueType::CProc:
            os << "CProcedure";
            break;

        case ValueType::List:
        {
            os << "[";
            for (auto it = V.constList().begin(), it_end = V.constList().end(); it != it_end; ++it)
            {
                if (it->valueType() == ValueType::String)
                    os << "\"" << (*it) << "\"";
                else
                    os << (*it);
                if (it + 1 != it_end)
                    os << " ";
            }
            os << "]";
            break;
        }

        case ValueType::Closure:
            os << V.closure();
            break;

        case ValueType::User:
            os << V.usertype();
            break;

        case ValueType::Nil:
            os << "nil";
            break;

        case ValueType::True:
            os << "true";
            break;

        case ValueType::False:
            os << "false";
            break;

        case ValueType::Undefined:
            os << "undefined";
            break;

        case ValueType::Reference:
            os << (*V.reference());
            break;

        case ValueType::InstPtr:
            os << "Instruction @ " << V.pageAddr();
            break;

        default:
            os << "~\\._./~";
            break;
        }

        return os;
    }
}