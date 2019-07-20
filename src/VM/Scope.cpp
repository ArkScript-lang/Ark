#include <Ark/VM/Scope.hpp>

#include <Ark/VM/FFI.hpp>

namespace Ark::internal
{
    unsigned Scope::id_generator = 0;

    Scope::Scope() :
        m_id(Scope::id_generator++),
        m_owned_by_vm(true),
        m_ref_count(1)
    {}

    Scope::Scope(std::size_t count, bool owned_by_vm) :
        m_id(Scope::id_generator++),
        m_locals(count, internal::FFI::undefined),
        m_owned_by_vm(owned_by_vm),
        m_ref_count(1)
    {}
}