#include <Ark/Compiler/ValTableElem.hpp>

namespace Ark::internal
{
    ValTableElem::ValTableElem(double num) noexcept :
        value(num),
        type(ValTableElemType::Number)
    {}

    ValTableElem::ValTableElem(const long num) noexcept :
        value(static_cast<double>(num)),
        type(ValTableElemType::Number)
    {}

    ValTableElem::ValTableElem(const std::string& str) noexcept :
        value(str),
        type(ValTableElemType::String)
    {}

    ValTableElem::ValTableElem(const Node& node) noexcept
    {
        if (node.nodeType() == NodeType::Number)
        {
            value = node.number();
            type = ValTableElemType::Number;
        }
        else if (node.nodeType() == NodeType::String)
        {
            value = node.string();
            type = ValTableElemType::String;
        }
    }

    ValTableElem::ValTableElem(std::size_t page) noexcept :
        value(page),
        type(ValTableElemType::PageAddr)
    {}

    bool ValTableElem::operator==(const ValTableElem& A) const noexcept
    {
        return A.value == value && A.type == type;
    }
}
