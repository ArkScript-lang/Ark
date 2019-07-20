#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Scope.hpp>

namespace Ark::internal
{
    Closure::Closure() :
        m_scope(nullptr),
        m_page_addr(0)
    {}

    Closure::Closure(Scope* scope_ptr, PageAddr_t pa) :
        m_scope(scope_ptr),
        m_page_addr(pa)
    {}

    Scope* Closure::scope()
    {
        return m_scope;
    }
}