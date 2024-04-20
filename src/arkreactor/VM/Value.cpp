#include <Ark/VM/Value.hpp>

#include <fmt/format.h>

#define init_const_type(is_const, type) (((is_const) ? (1 << 7) : 0) | static_cast<uint8_t>(type))

namespace Ark
{
    Value::Value() noexcept :
        m_const_type(init_const_type(false, ValueType::Undefined))
    {}

    Value::Value(ValueType type) noexcept :
        m_const_type(init_const_type(false, type))
    {
        if (type == ValueType::List)
            m_value = std::vector<Value>();
        else if (type == ValueType::String)
            m_value = "";
    }

    Value::Value(int value) noexcept :
        m_const_type(init_const_type(false, ValueType::Number)), m_value(static_cast<double>(value))
    {}

    Value::Value(float value) noexcept :
        m_const_type(init_const_type(false, ValueType::Number)), m_value(static_cast<double>(value))
    {}

    Value::Value(double value) noexcept :
        m_const_type(init_const_type(false, ValueType::Number)), m_value(value)
    {}

    Value::Value(const std::string& value) noexcept :
        m_const_type(init_const_type(false, ValueType::String)), m_value(value)
    {}

    Value::Value(const char* value) noexcept :
        m_const_type(init_const_type(false, ValueType::String)), m_value(value)
    {}

    Value::Value(internal::PageAddr_t value) noexcept :
        m_const_type(init_const_type(false, ValueType::PageAddr)), m_value(value)
    {}

    Value::Value(Value::ProcType value) noexcept :
        m_const_type(init_const_type(false, ValueType::CProc)), m_value(value)
    {}

    Value::Value(std::vector<Value>&& value) noexcept :
        m_const_type(init_const_type(false, ValueType::List)), m_value(std::move(value))
    {}

    Value::Value(internal::Closure&& value) noexcept :
        m_const_type(init_const_type(false, ValueType::Closure)), m_value(std::move(value))
    {}

    Value::Value(UserType&& value) noexcept :
        m_const_type(init_const_type(false, ValueType::User)), m_value(value)
    {}

    Value::Value(Value* ref) noexcept :
        m_const_type(init_const_type(true, ValueType::Reference)), m_value(ref)
    {}

    // --------------------------

    std::vector<Value>& Value::list()
    {
        return std::get<std::vector<Value>>(m_value);
    }

    internal::Closure& Value::refClosure()
    {
        return std::get<internal::Closure>(m_value);
    }

    std::string& Value::stringRef()
    {
        return std::get<std::string>(m_value);
    }

    UserType& Value::usertypeRef()
    {
        return std::get<UserType>(m_value);
    }

    Value* Value::reference() const
    {
        return std::get<Value*>(m_value);
    }

    // --------------------------

    void Value::push_back(const Value& value)
    {
        list().push_back(value);
    }

    void Value::push_back(Value&& value)
    {
        list().push_back(std::move(value));
    }


    void Value::toString(std::ostream& os, VM& vm) const noexcept
    {
        switch (valueType())
        {
            case ValueType::Number:
            {
                double d = number();
                os << fmt::format("{}", d);
                break;
            }

            case ValueType::String:
                os << string();
                break;

            case ValueType::PageAddr:
                os << "Function @ " << pageAddr();
                break;

            case ValueType::CProc:
                os << "CProcedure";
                break;

            case ValueType::List:
            {
                os << "[";
                for (auto it = constList().begin(), it_end = constList().end(); it != it_end; ++it)
                {
                    if (it->valueType() == ValueType::String)
                    {
                        os << "\"";
                        it->toString(os, vm);
                        os << "\"";
                    }
                    else
                        it->toString(os, vm);
                    if (it + 1 != it_end)
                        os << " ";
                }
                os << "]";
                break;
            }

            case ValueType::Closure:
                closure().toString(os, vm);
                break;

            case ValueType::User:
                os << usertype();
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
                reference()->toString(os, vm);
                break;

            case ValueType::InstPtr:
                os << "Instruction @ " << pageAddr();
                break;

            default:
                os << "~\\._./~";
                break;
        }
    }
}
