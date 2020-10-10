#include <Ark/Compiler/CValue.hpp>

namespace Ark::internal
{
    CValue::CValue(double value) noexcept :
        value(value),
        type(CValueType::Number)
    {}

    CValue::CValue(long value) noexcept :
        value(static_cast<double>(value)),
        type(CValueType::Number)
    {}

    CValue::CValue(const std::string& value) noexcept :
        value(value),
        type(CValueType::String)
    {}

    CValue::CValue(const Node& v) noexcept
    {
        if (v.nodeType() == NodeType::Number)
        {
            value = v.number();
            type = CValueType::Number;
        }
        else if (v.nodeType() == NodeType::String)
        {
            value = v.string();
            type = CValueType::String;
        }
    }

    CValue::CValue(std::size_t value) noexcept :
        value(value),
        type(CValueType::PageAddr)
    {}

    bool CValue::operator==(const CValue& A) noexcept
    {
        return A.value == value && A.type == type;
    }
}