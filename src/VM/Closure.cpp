#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Scope.hpp>

namespace Ark::internal
{
    Closure::Closure() :
        m_scope(nullptr),
        m_page_addr(0)
    {}

    Closure::Closure(Scope_t&& scope_ptr, PageAddr_t pa) :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    Closure::Closure(const Scope_t& scope_ptr, PageAddr_t pa) :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    const Scope_t& Closure::scope() const
    {
        return m_scope;
    }

    Scope_t& Closure::scope_ref()
    {
        return m_scope;
    }

    std::ostream& operator<<(std::ostream& os, const Closure& C)
    {
        os << "Closure<" << C.m_page_addr << ">";
        return os;
    }
}