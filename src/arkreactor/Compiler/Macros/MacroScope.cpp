#include <Ark/Compiler/Macros/MacroScope.hpp>

namespace Ark::internal
{
    MacroScope::MacroScope() :
        m_depth(0)
    {}

    MacroScope::MacroScope(unsigned int depth) :
        m_depth(depth)
    {}

    const Node* MacroScope::has(const std::string& name) const
    {
        if (auto res = m_macros.find(name); res != m_macros.end())
            return &res->second;
        return nullptr;
    }

    void MacroScope::add(const std::string& name, const Node& node)
    {
        m_macros[name] = node;
    }

    bool MacroScope::remove(const std::string& name)
    {
        if (auto res = m_macros.find(name); res != m_macros.end())
        {
            m_macros.erase(res);
            return true;
        }
        return false;
    }
}
