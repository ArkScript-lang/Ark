#include <Ark/Compiler/Value.hpp>

namespace Ark
{
    namespace Compiler
    {
        using namespace Ark::Lang;

        Value::Value(int value) :
            value(BigNum(value)),
            type(ValueType::Number)
        {}

        Value::Value(long value) :
            value(BigNum(value)),
            type(ValueType::Number)
        {}

        Value::Value(const std::string& value) :
            value(value),
            type(ValueType::String)
        {}

        Value::Value(const Node& v)
        {
            if (v.nodeType() == NodeType::Number)
            {
                value = v.getIntVal();
                type = ValueType::Number;
            }
            else if (v.nodeType() == NodeType::String)
            {
                value = v.getStringVal();
                type = ValueType::String;
            }
        }

        Value::Value(std::size_t value) :
            value(value),
            type(ValueType::PageAddr)
        {}

        bool Value::operator==(const Value& A)
        {
            return A.value == value && A.type == type;
        }
    }
}