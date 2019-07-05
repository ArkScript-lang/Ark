#ifndef ark_vm_closure
#define ark_vm_closure

#include <Ark/VM/Types.hpp>

namespace Ark::internal
{
    // TODO store in Value directly
    class Closure
    {
    public:
        Closure();
        Closure(PageAddr_t pa);

        PageAddr_t pageAddr() const;

        friend inline bool operator==(const Closure& A, const Closure& B);
    
    private:
        PageAddr_t m_page_addr;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_page_addr == B.m_page_addr;
    }
}

#endif