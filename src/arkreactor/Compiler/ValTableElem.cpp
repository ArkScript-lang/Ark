#include <Ark/Compiler/ValTableElem.hpp>

namespace Ark::internal
{
    ValTableElem::ValTableElem(double value) noexcept :
        value(value),
        type(ValTableElemType::Number)
    {}

    ValTableElem::ValTableElem(long value) noexcept :
        value(static_cast<double>(value)),
        type(ValTableElemType::Number)
    {}

    ValTableElem::ValTableElem(const std::string& value) noexcept :
        value(value),
        type(ValTableElemType::String)
    {}

    ValTableElem::ValTableElem(const Node& v) noexcept
    {
        if (v.nodeType() == NodeType::Number)
        {
            value = v.number();
            type = ValTableElemType::Number;
        }
        else if (v.nodeType() == NodeType::String)
        {
            value = v.string();
            type = ValTableElemType::String;
        }
    }

    ValTableElem::ValTableElem(std::size_t value) noexcept :
        value(value),
        type(ValTableElemType::PageAddr)
    {}

    bool ValTableElem::operator==(const ValTableElem& A) noexcept
    {
        return A.value == value && A.type == type;
    }
}
