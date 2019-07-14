#ifndef ark_vm_closure
#define ark_vm_closure

#include <memory>
#include <vector>

#include <Ark/VM/Types.hpp>

namespace Ark::internal
{
    class Value;

    class Closure
    {
    public:
        using Scope_t = std::shared_ptr<std::vector<Value>>;

        Closure();
        Closure(Scope_t&& scope_ptr, PageAddr_t pa);
        Closure(const Scope_t& scope_ptr, PageAddr_t pa);

        const Scope_t& scope() const;
        Scope_t& scope_ref();

        inline PageAddr_t pageAddr() const
        {
            return m_page_addr;
        }

        friend inline bool operator==(const Closure& A, const Closure& B);
    
    private:
        Scope_t m_scope;
        PageAddr_t m_page_addr;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_scope == B.m_scope;
    }
}

#endif