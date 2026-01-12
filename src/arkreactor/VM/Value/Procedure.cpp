#include <Ark/VM/Value/Value.hpp>
#include <Ark/VM/Value/Procedure.hpp>

#include <utility>

namespace Ark
{
    Value Procedure::operator()(std::vector<Value>& args, VM* vm) const
    {
        return m_procedure(args, vm);
    }

    Procedure::Procedure(PointerType c_pointer) :
        m_procedure(c_pointer)
    {}

    bool Procedure::operator<(const Procedure&) const noexcept
    {
        return false;
    }

    bool Procedure::operator==(const Procedure&) const noexcept
    {
        return false;
    }
};
