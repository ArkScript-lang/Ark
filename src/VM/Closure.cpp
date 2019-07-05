#include <Ark/VM/Closure.hpp>

namespace Ark::internal
{
    Closure::Closure() :
        m_page_addr(0)
    {}

    Closure::Closure(PageAddr_t pa) :
        m_page_addr(pa)
    {}

    PageAddr_t Closure::pageAddr() const
    {
        return m_page_addr;
    }
}