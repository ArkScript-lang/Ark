#include <Ark/VM/Value/Closure.hpp>

#include <Ark/VM/Value/ClosureScope.hpp>
#include <Ark/VM/VM.hpp>

#include <ranges>

namespace Ark::internal
{
    // This can cause a memory leak if a closure is referencing itself.
    // However, I don't think this is worthy to try and fix, I spent far
    // too much time (~10 days, 4 different solutions) and energy on it,
    // and no solution was good enough, they all had pretty big flaws:
    // 1. keeping the closures in a central place and reference them using
    //    raw pointers : need GC or have a growing memory that will be
    //    fred only at the end).
    // 2. use a generational storage, one scope = one generation, free old
    //    gens when we create a new one. Alas we can create a closure in a
    //    gen and return it in another, so it would have created dangling refs.
    // 3. use a variant weak/shared and copy as weak ptr, the leak is still
    //    there.
    // 4. like 1, put all shared ptrs in the execution context and use weak ref,
    //    it needs a GC or the memory will grow until the execution context is
    //    deleted.
    // This problem is being ignored by the CI, via lsan-suppressions.txt and
    // valgrind-suppressions.txt.
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
        return std::ranges::any_of(std::ranges::views::keys(m_scope->m_data), [&vm, &end](const auto& id) {
            return end.ends_with(":" + vm.m_state.m_symbols[id]);
        });
    }

    std::string Closure::toString(VM& vm) const noexcept
    {
        std::string out = "(";
        for (std::size_t i = 0, end = m_scope->m_data.size(); i < end; ++i)
        {
            const auto& [id, value] = m_scope->m_data[i];
            if (i != 0)
                out += ' ';

            out += '.' + vm.m_state.m_symbols[id] + '=';
            if (value.valueType() == ValueType::Closure && value.closure().scopePtr() == scopePtr())
                out += "Ref(self)";
            else
                out += value.toString(vm);
        }
        return out + ")";
    }

    bool operator==(const Closure& A, const Closure& B) noexcept
    {
        // they do not come from the same closure builder
        if (A.m_page_addr != B.m_page_addr)
            return false;
        // pointers are identical, we are dealing with the same object
        if (A.m_scope.get() == B.m_scope.get())
            return true;

        return *A.m_scope == *B.m_scope;
    }
}
