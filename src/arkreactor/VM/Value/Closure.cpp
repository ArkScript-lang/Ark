#include <Ark/VM/Value/Closure.hpp>

#include <Ark/VM/Value/ClosureScope.hpp>
#include <Ark/VM/Value.hpp>
#include <Ark/VM/VM.hpp>

#include <ranges>

namespace Ark::internal
{
    Closure::Closure(const ClosureScope& scope, const PageAddr_t pa) noexcept :
        m_scope(std::make_shared<ClosureScope>(scope)),
        m_page_addr(pa)
    {}

    Closure::Closure(const std::shared_ptr<ClosureScope>& scope_ptr, const PageAddr_t pa) noexcept :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    bool Closure::hasFieldEndingWith(const std::string& end, const VM& vm) const
    {
        for (const auto id : std::ranges::views::keys(m_scope->m_data))
        {
            if (end.ends_with(":" + vm.m_state.m_symbols[id]))
                return true;
        }
        return false;
    }

    std::string Closure::toString(VM& vm) const noexcept
    {
        std::string out = "(";
        for (std::size_t i = 0, end = m_scope->m_data.size(); i < end; ++i)
        {
            if (i != 0)
                out += ' ';

            out += '.' + vm.m_state.m_symbols[m_scope->m_data[i].first] + '=';
            out += m_scope->m_data[i].second.toString(vm);
        }
        return out + ")";
    }

    bool operator==(const Closure& A, const Closure& B) noexcept
    {
        // they do not come from the same closure builder
        if (A.m_page_addr != B.m_page_addr)
            return false;

        return *A.m_scope == *B.m_scope;
    }
}
