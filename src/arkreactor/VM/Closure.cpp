#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Scope.hpp>

namespace Ark::internal
{
    Closure::Closure() noexcept :
        m_scope(nullptr),
        m_page_addr(0)
    {}

    Closure::Closure(Scope_t&& scope_ptr, PageAddr_t pa) noexcept :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    Closure::Closure(const Scope_t& scope_ptr, PageAddr_t pa) noexcept :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    const Scope_t& Closure::scope() const noexcept
    {
        return m_scope;
    }

    Scope_t& Closure::refScope() noexcept
    {
        return m_scope;
    }

    std::ostream& operator<<(std::ostream& os, const Closure& C) noexcept
    {
        os << "Closure<" << C.m_page_addr << ">";
        return os;
    }
}