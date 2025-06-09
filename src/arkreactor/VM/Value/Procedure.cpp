#include <Ark/VM/Value.hpp>
#include <Ark/VM/Value/Procedure.hpp>

namespace Ark
{
    Procedure::Procedure(const CallbackType& functor) noexcept :
        m_procedure(functor)
    {
    }

    Procedure::Procedure(CallbackType&& functor) noexcept :
        m_procedure(functor)
    {
    }

    Value Procedure::operator()(std::vector<Value>& args, VM* vm) const
    {
        return m_procedure(args, vm);
    }

    bool Procedure::operator<(const Procedure&) const noexcept
    {
        return false;
    }

    bool Procedure::operator==(const Procedure&) const noexcept
    {
        return false;
    }

};
