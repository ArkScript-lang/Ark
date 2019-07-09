#include <Ark/VM/Closure.hpp>

#include <Ark/VM/Value.hpp>

namespace Ark::internal
{
    Closure::Closure() :
        m_page_addr(0)
    {}

    Closure::Closure(PageAddr_t pa, Scope_t::iterator begin, Scope_t::iterator end) :
        m_page_addr(pa)
    {
        while (begin != end)
        {
            m_binded_vars.push_back(*begin);
            begin++;
        }
    }

    const Closure::Scope_t& Closure::bindedVars() const
    {
        return m_binded_vars;
    }

    void Closure::save(Scope_t::iterator begin, Scope_t::iterator end)
    {
        while (begin != end)
        {
            for (auto& id_val: m_binded_vars)
            {
                if (id_val.first == begin->first)
                    id_val.second = begin->second;
            }
            begin++;
        }
    }
}