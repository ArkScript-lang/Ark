#include <Ark/Compiler/Value.hpp>

#include <Ark/Lang/Node.hpp>

namespace Ark
{
    namespace Compiler
    {
        using namespace Ark::Lang;

        template <> Value::Value(const int& value) :
            value(dozerg::HugeNumber(value)),
            type(ValueType::Number)
        {}

        template <> Value::Value(const long& value) :
            value(dozerg::HugeNumber(value)),
            type(ValueType::Number)
        {}

        template <> Value::Value(const std::string& value) :
            value(value),
            type(ValueType::String)
        {}

        template <> Value::Value(const Node& v)
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

        template <> Value::Value(const std::size_t& value) :
            value(value),
            type(ValueType::PageAddr)
        {}

        bool Value::operator==(const Value& A)
        {
            return A.value == value && A.type == type;
        }
    }
}