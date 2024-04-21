#include <Ark/VM/Value.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

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

    void Value::push_back(const Value& value)
    {
        list().emplace_back(value);
    }

    void Value::push_back(Value&& value)
    {
        list().emplace_back(std::move(value));
    }

    std::string Value::toString(VM& vm) const noexcept
    {
        switch (valueType())
        {
            case ValueType::Number:
                return fmt::format("{}", number());

            case ValueType::String:
                return string();

            case ValueType::PageAddr:
                return fmt::format("Function @ {}", pageAddr());

            case ValueType::CProc:
                return "CProcedure";

            case ValueType::List:
            {
                std::string out = "[";
                for (auto it = constList().begin(), it_end = constList().end(); it != it_end; ++it)
                {
                    if (it->valueType() == ValueType::String)
                        out += "\"" + it->toString(vm) + "\"";
                    else
                        out += it->toString(vm);
                    if (it + 1 != it_end)
                        out += " ";
                }
                return out + "]";
            }

            case ValueType::Closure:
                return closure().toString(vm);

            case ValueType::User:
                return fmt::format("{}", fmt::streamed(usertype()));

            case ValueType::Nil:
                return "nil";

            case ValueType::True:
                return "true";

            case ValueType::False:
                return "false";

            case ValueType::Undefined:
                return "undefined";

            case ValueType::Reference:
                return reference()->toString(vm);

            case ValueType::InstPtr:
                return fmt::format("Instruction @ {}", pageAddr());

            default:
                return "~\\._./~";
        }
    }
}
