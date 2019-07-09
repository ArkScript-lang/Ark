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

        inline PageAddr_t pageAddr() const
        {
            return m_page_addr;
        }

        const Scope_t& bindedVars() const;
        void save(Scope_t::iterator begin, Scope_t::iterator end);

        friend inline bool operator==(const Closure& A, const Closure& B);
    
    private:
        PageAddr_t m_page_addr;
        Scope_t m_binded_vars;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_page_addr == B.m_page_addr;
    }
}

#endif