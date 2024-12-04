#include <Ark/Compiler/NameResolution/StaticScope.hpp>

#include <utility>
#include <fmt/format.h>

namespace Ark::internal
{
    void StaticScope::add(const std::string& name, bool is_mutable)
    {
        m_vars.emplace(name, is_mutable);
    }

    std::optional<Declaration> StaticScope::get(const std::string& name, [[maybe_unused]] const bool extensive_lookup) const
    {
        if (const auto it = std::ranges::find(m_vars, name, &Declaration::name); it != m_vars.end())
            return *it;
        return std::nullopt;
    }

    std::string StaticScope::fullyQualifiedName(const std::string& name) const
    {
        return name;
    }

    bool StaticScope::saveNamespace([[maybe_unused]] std::unique_ptr<StaticScope>&)
    {
        // the scope can not be saved on a static scope
        return false;
    }

    bool StaticScope::isNamespace() const
    {
        return false;
    }

    NamespaceScope::NamespaceScope(std::string name, const bool with_prefix, const bool is_glob, const std::vector<std::string>& symbols) :
        StaticScope(),
        m_namespace(std::move(name)),
        m_with_prefix(with_prefix),
        m_is_glob(is_glob),
        m_symbols(symbols)
    {}

    void NamespaceScope::add(const std::string& name, bool is_mutable)
    {
        m_vars.emplace(fullyQualifiedName(name), is_mutable);
    }

    std::optional<Declaration> NamespaceScope::get(const std::string& name, const bool extensive_lookup) const
    {
        const bool starts_with_prefix = !m_namespace.empty() && name.starts_with(m_namespace + ":");
        // If the name starts with the namespace and we imported the namespace with prefix
        // search for name in the namespace
        if (starts_with_prefix && m_with_prefix)
        {
            if (const auto it = std::ranges::find(m_vars, name, &Declaration::name); it != m_vars.end())
                return *it;
        }
        // If the name does not start with the prefix, and we import through either glob or symbol list
        // search for the name in the namespace, while adding the namespace in front (as we use fully
        // qualified names when registering declarations).
        // If the name wasn't qualified, in a prefixed namespace, look up for it but by qualifying the name
        else if (!starts_with_prefix && (m_is_glob || std::ranges::find(m_symbols, name) != m_symbols.end() || m_with_prefix))
        {
            if (const auto it_fqn = std::ranges::find(m_vars, fullyQualifiedName(name), &Declaration::name); it_fqn != m_vars.end())
                return *it_fqn;
        }
        // lookup in the additional saved namespaces
        if (extensive_lookup)
        {
            for (const auto& scope : m_additional_namespaces)
            {
                if (auto maybe_decl = scope->get(name, extensive_lookup); maybe_decl.has_value())
                    return maybe_decl;
            }
        }
        // otherwise we didn't find the name in the namespace
        return std::nullopt;
    }

    std::string NamespaceScope::fullyQualifiedName(const std::string& name) const
    {
        const bool starts_with_prefix = !m_namespace.empty() && name.starts_with(m_namespace + ":");
        if (!m_namespace.empty() && !starts_with_prefix)
            return fmt::format("{}:{}", m_namespace, name);
        return name;
    }

    bool NamespaceScope::saveNamespace(std::unique_ptr<StaticScope>& scope)
    {
        m_additional_namespaces.push_back(std::move(scope));
        return true;
    }

    bool NamespaceScope::isNamespace() const
    {
        return true;
    }
}
