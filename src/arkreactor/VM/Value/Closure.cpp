#include <Ark/VM/Value/Closure.hpp>

#include <Ark/VM/Scope.hpp>
#include <Ark/VM/Value.hpp>
#include <Ark/VM/VM.hpp>

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

    void Closure::toString(std::ostream& os, VM& vm) const noexcept
    {
        os << "(";
        for (std::size_t i = 0, end = m_scope->m_data.size(); i < end; ++i)
        {
            if (i != 0)
                os << ' ';

            os << '.' << vm.m_state.m_symbols[m_scope->m_data[i].first] << '=';
            m_scope->m_data[i].second.toString(os, vm);
        }
        os << ")";
    }

    bool operator==(const Closure& A, const Closure& B) noexcept
    {
        // they do not come from the same closure builder
        if (A.m_page_addr != B.m_page_addr)
            return false;

        return *A.m_scope == *B.m_scope;
    }
}
