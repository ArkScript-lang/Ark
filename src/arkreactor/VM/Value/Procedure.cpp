#include <Ark/VM/Value.hpp>
#include <Ark/VM/Value/Procedure.hpp>

#include <utility>

namespace Ark
{
    Value Procedure::operator()(std::vector<Value>& args, VM* vm) const
    {
        return m_procedure(args, vm);
    }

    Procedure::Procedure(PointerType c_pointer)
    {
        m_procedure = c_pointer;
    }

    Procedure::Procedure(Procedure&& other) :
        m_procedure(std::exchange(other.m_procedure, nullptr)) {}

    Procedure::Procedure(const Procedure& other) :
        m_procedure(other.m_procedure)
    {
    }

    Procedure& Procedure::operator=(Procedure&& other)
    {
        m_procedure = std::move(other.m_procedure);
        return *this;
    }

    Procedure& Procedure::operator=(const Procedure& other)
    {
        m_procedure = other.m_procedure;
        return *this;
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
