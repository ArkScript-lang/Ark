#ifndef ark_vm_closure
#define ark_vm_closure

#include <vector>
#include <utility>

#include <Ark/VM/Types.hpp>

namespace Ark::internal
{
    class Value;

    class Closure
    {
    public:
        using Scope_t = std::vector<std::pair<uint16_t, internal::Value>>;

        Closure();
        Closure(PageAddr_t pa, Scope_t::iterator begin, Scope_t::iterator end);

        const Scope_t& bindedVars() const;

        friend inline bool operator==(const Closure& A, const Closure& B);
    
    private:
        Scope_t m_binded_vars;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_page_addr == B.m_page_addr;
    }
}

#endif