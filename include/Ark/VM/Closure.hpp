#ifndef ark_vm_closure
#define ark_vm_closure

#include <memory>
#include <vector>
#include <iostream>

#include <Ark/VM/Types.hpp>

namespace Ark::internal
{
    class Value;

    /*
        A scope is defined as a shared pointer to a list of local variables
        because a Closure could continue to leave when the local variables list
        has been closed by the virtual machine
    */
    using Scope_t = std::shared_ptr<std::vector<Value>>;

    class Closure
    {
    public:
        Closure();
        Closure(Scope_t&& scope_ptr, PageAddr_t pa);
        Closure(const Scope_t& scope_ptr, PageAddr_t pa);

        // getters
        const Scope_t& scope() const;
        Scope_t& scope_ref();

        inline PageAddr_t pageAddr() const { return m_page_addr; }

        friend inline bool operator==(const Closure& A, const Closure& B);
        friend inline bool operator<(const Closure& A, const Closure& B);
        friend std::ostream& operator<<(std::ostream& os, const Closure& C);
    
    private:
        Scope_t m_scope;
        // keep track of the code page number, in case we need it later
        PageAddr_t m_page_addr;
    };

    inline bool operator==(const Closure& A, const Closure& B)
    {
        return A.m_scope == B.m_scope;
    }

    inline bool operator<(const Closure& A, const Closure& B)
    {
        return A.m_page_addr < B.m_page_addr;
    }
}

#endif