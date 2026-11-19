#include <Ark/Compiler/NameResolution/StaticScope.hpp>

#include <ranges>
#include <utility>
#include <algorithm>
#include <fmt/format.h>

#include <Ark/Compiler/Common.hpp>

namespace Ark::internal
{
    std::string StaticScope::add(const std::string& name, bool is_mutable)
    {
        m_vars.emplace(name, name, is_mutable);
        return name;
    }

    std::optional<Declaration> StaticScope::get(const std::string& name, [[maybe_unused]] const std::string& origin_namespace)
    {
        if (const auto it = std::ranges::find(m_vars, name, &Declaration::name); it != m_vars.end())
            return *it;
        return std::nullopt;
    }

    std::string StaticScope::fullyQualifiedName(const std::string& name) const
    {
        return name;
    }

    bool StaticScope::isNamespace() const
    {
        return false;
    }

    NamespaceScope::NamespaceScope(std::string prefix, const std::string& package, const std::vector<Import>& imports) :
        StaticScope(),
        m_namespace(std::move(prefix)),
        m_package(package),
        m_imports(imports)
    {}

    std::string NamespaceScope::add(const std::string& name, bool is_mutable)
    {
        const std::string fqn = fullyQualifiedName(name);
        return m_vars.emplace(fqn, name, is_mutable).first->name;
    }

    std::optional<Declaration> NamespaceScope::get(const std::string& name, const std::string& origin_namespace)
    {
        if (!m_namespace.empty() && name.starts_with(m_namespace + ":"))
        {
            if (const auto it = std::ranges::find(m_vars, name, &Declaration::name); it != m_vars.end())
                return *it;
        }
        // If the name does not start with the prefix, and we import through either glob or symbol list,
        // Or if the name does not start with the prefix, in a namespace with a symbol list but can be resolved,
        // Or if the name wasn't qualified, in the current prefixed namespace,
        // search for the qualified name in the namespace
        else
        {
            const auto fqn = fullyQualifiedName(name);

            auto it = m_vars.end();
            auto it_original = m_vars.end();
            for (auto j = m_vars.begin(), end = m_vars.end(); j != end; ++j)
            {
                if (j->name == fqn)
                    it = j;
                if (j->original_name == name)
                    it_original = j;

                if (it != end && it_original != end)
                    break;
            }

            if (origin_namespace == m_namespace && it != m_vars.end())
                return *it;
            if (it_original != m_vars.end())
                return *it_original;
        }
        // otherwise we didn't find the name in the namespace
        return std::nullopt;
    }

    std::string NamespaceScope::fullyQualifiedName(const std::string& name) const
    {
        if (m_namespace.empty() || name.starts_with(m_namespace + ":"))
            return name;
        return fmt::format("{}:{}", m_namespace, name);
    }

    bool NamespaceScope::isNamespace() const
    {
        return true;
    }
}
