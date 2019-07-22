#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Value.hpp>

namespace Ark::internal
{
    Closure::Closure() :
        m_scope(nullptr),
        m_page_addr(0)
    {}

    Closure::Closure(Closure::Scope_t&& scope_ptr, PageAddr_t pa) :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    Closure::Closure(const Closure::Scope_t& scope_ptr, PageAddr_t pa) :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    const Closure::Scope_t& Closure::scope() const
    {
        return m_scope;
    }

    Closure::Scope_t& Closure::scope_ref()
    {
        return m_scope;
    }
}