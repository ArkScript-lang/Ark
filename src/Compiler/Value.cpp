#include <Ark/Compiler/Value.hpp>

#include <Ark/Lang/Node.hpp>

namespace Ark
{
    namespace Compiler
    {
        using namespace Ark::Lang;

        template <> Value(const int& value) :
            value(dozerg::HugeNumber(value)),
            type(ValueType::Number)
        {}

        template <> Value(const long& value) :
            value(dozerg::HugeNumber(value)),
            type(ValueType::Number)
        {}

        template <> Value(const std::string& value) :
            value(value),
            type(ValueType::String)
        {}

        template <> Value(const Node& v)
        {
            if (v.nodeType() == NodeType::Number)
                value = v.getIntVal();
            else if (v.nodeType() == NodeType::String)
                value = v.getStringVal();
        }

        bool Value::operator==(const Value& A)
        {
            return A.value == value && A.type == type;
        }
    }
}