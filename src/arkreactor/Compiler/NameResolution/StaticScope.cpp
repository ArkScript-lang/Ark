#include <Ark/Compiler/NameResolution/StaticScope.hpp>

#include <utility>
#include <algorithm>
#include <fmt/format.h>

namespace Ark::internal
{
    std::string StaticScope::add(const std::string& name, bool is_mutable)
    {
        m_vars.emplace(name, name, is_mutable);
        return name;
    }

    std::optional<Declaration> StaticScope::get(const std::string& name, [[maybe_unused]] const std::string& origin_namespace, [[maybe_unused]] const bool extensive_lookup)
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

    std::string NamespaceScope::add(const std::string& name, bool is_mutable)
    {
        // Since we do multiple passes on namespaces, we need to check if the given name is already hidden,
        // so that we can save the name as it was on the first pass
        if (name.ends_with("#hidden"))
        {
            std::string std_name = name.substr(0, name.find_first_of('#'));
            return m_vars.emplace(name, std_name, is_mutable).first->name;
        }

        // Otherwise, we also have to check for the presence of a namespace prefix,
        // and remove it when checking against the symbols list, to determine if we
        // need to hide the name or not
        const bool starts_with_prefix = !m_namespace.empty() && name.starts_with(m_namespace + ":");
        const std::string fqn = fullyQualifiedName(name);
        const std::string unprefixed_name = starts_with_prefix ? name.substr(name.find_first_of(':') + 1) : name;

        if (!m_symbols.empty() && !hasSymbol(unprefixed_name) && !m_with_prefix && !m_is_glob)
            return m_vars.emplace(fqn + "#hidden", fqn, is_mutable).first->name;
        return m_vars.emplace(fqn, fqn, is_mutable).first->name;
    }

    std::optional<Declaration> NamespaceScope::get(const std::string& name, const std::string& origin_namespace, const bool extensive_lookup)
    {
        const bool starts_with_prefix = !m_namespace.empty() && name.starts_with(m_namespace + ":");
        // If the name starts with the namespace, and we imported the namespace with prefix
        // search for name in the namespace
        if (starts_with_prefix && m_with_prefix)
        {
            if (const auto it = std::ranges::find(m_vars, name, &Declaration::name); it != m_vars.end())
                return *it;
        }
        // If the name does not start with the prefix, and we import through either glob or symbol list
        // search for the name in the namespace
        // If the name does not start with the prefix, in a namespace with a symbol list but can be resolved,
        // modify it to hide it to the end user
        // If the name wasn't qualified, in the current prefixed namespace, look up for it but by qualifying the name
        else if (!starts_with_prefix)
        {
            const auto it = std::ranges::find(m_vars, fullyQualifiedName(name), &Declaration::name);
            const auto it_original = std::ranges::find(m_vars, fullyQualifiedName(name), &Declaration::original_name);
            if ((m_is_glob || hasSymbol(name) || (m_with_prefix && origin_namespace == m_namespace)) && it != m_vars.end())
                return *it;
            if (!m_symbols.empty() && it_original != m_vars.end())
                return *it_original;
        }
        // lookup in the additional saved namespaces
        if (extensive_lookup)
        {
            std::optional<Declaration> decl;
            for (const auto& scope : m_additional_namespaces)
            {
                if (auto maybe_decl = scope->get(name, origin_namespace, extensive_lookup); maybe_decl.has_value())
                {
                    // prioritize non-hidden declarations
                    if ((decl.has_value() && decl.value().name.ends_with("#hidden")) || !decl.has_value())
                        decl = maybe_decl;
                }
            }
            return decl;
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

    bool NamespaceScope::recursiveHasSymbol(const std::string& symbol)
    {
        if (hasSymbol(symbol))
            return true;
        if (isGlob() && std::ranges::find(m_vars, fullyQualifiedName(symbol), &Declaration::name) != m_vars.end())
            return true;

        return std::ranges::any_of(
            m_additional_namespaces,
            [&symbol](const auto& saved_scope) {
                return saved_scope->recursiveHasSymbol(symbol);
            });
    }
}
