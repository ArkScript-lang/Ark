#include <Ark/Compiler/Macros/MacroScope.hpp>

namespace Ark::internal
{
    MacroScope::MacroScope(const unsigned int depth) :
        m_depth(depth)
    {}

    const Node* MacroScope::has(const std::string& name) const
    {
        if (const auto res = m_macros.find(name); res != m_macros.end())
            return &res->second;
        return nullptr;
    }

    void MacroScope::add(const std::string& name, const Node& node)
    {
        m_macros[name] = node;
    }

    bool MacroScope::remove(const std::string& name)
    {
        if (const auto res = m_macros.find(name); res != m_macros.end())
        {
            m_macros.erase(res);
            return true;
        }
        return false;
    }
}
