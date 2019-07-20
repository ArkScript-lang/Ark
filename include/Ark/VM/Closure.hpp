#ifndef ark_vm_closure
#define ark_vm_closure

#include <Ark/VM/Types.hpp>

namespace Ark::internal
{
    class Scope;

    class Closure
    {
    public:
        Closure();
        Closure(Scope* scope_ptr, PageAddr_t pa);

        Scope* scope();

        inline PageAddr_t pageAddr() const
        {
            return m_page_addr;
        }

        friend inline bool operator==(const Closure& A, const Closure& B);
    
    private:
        Scope* m_scope;
        PageAddr_t m_page_addr;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_scope == B.m_scope;
    }
}

#endif