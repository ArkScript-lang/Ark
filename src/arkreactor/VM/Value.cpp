#include <Ark/VM/Value.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace Ark
{
    Value::Value() noexcept :
        m_type(ValueType::Undefined)
    {}

    Value::Value(ValueType type) noexcept :
        m_type(type)
    {
        if (type == ValueType::List)
            m_value = std::vector<Value>();
        else if (type == ValueType::String)
            m_value = "";
    }

    Value::Value(const int value) noexcept :
        m_type(ValueType::Number), m_value(static_cast<double>(value))
    {}

    Value::Value(double value) noexcept :
        m_type(ValueType::Number), m_value(value)
    {}

    Value::Value(const std::string& value) noexcept :
        m_type(ValueType::String), m_value(value)
    {}

    Value::Value(internal::PageAddr_t value) noexcept :
        m_type(ValueType::PageAddr), m_value(value)
    {}

    Value::Value(Value::ProcType value) noexcept :
        m_type(ValueType::CProc), m_value(value)
    {}

    Value::Value(std::vector<Value>&& value) noexcept :
        m_type(ValueType::List), m_value(std::move(value))
    {}

    Value::Value(internal::Closure&& value) noexcept :
        m_type(ValueType::Closure), m_value(std::move(value))
    {}

    Value::Value(UserType&& value) noexcept :
        m_type(ValueType::User), m_value(value)
    {}

    Value::Value(Value* ref) noexcept :
        m_type(ValueType::Reference), m_value(ref)
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
