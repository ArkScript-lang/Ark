#include <Ark/Compiler/Value.hpp>

namespace Ark::internal
{
    CValue::CValue(double value) :
        value(value),
        type(CValueType::Number)
    {}

    CValue::CValue(long value) :
        value(static_cast<double>(value)),
        type(CValueType::Number)
    {}

    CValue::CValue(const std::string& value) :
        value(value),
        type(CValueType::String)
    {}

    CValue::CValue(const Node& v)
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

    CValue::CValue(std::size_t value) :
        value(value),
        type(CValueType::PageAddr)
    {}

    bool CValue::operator==(const CValue& A)
    {
        return A.value == value && A.type == type;
    }
}