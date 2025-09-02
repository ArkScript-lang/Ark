#include <Ark/Compiler/NameResolution/ScopeResolver.hpp>

#include <ranges>
#include <algorithm>

namespace Ark::internal
{
    ScopeResolver::ScopeResolver()
    {
        createNewNamespace("", /* with_prefix= */ false, /* is_glob= */ true, /* symbols= */ {});
    }

    void ScopeResolver::createNew()
    {
        m_scopes.emplace_back(std::make_unique<StaticScope>());
    }

    void ScopeResolver::removeLastScope()
    {
        m_scopes.pop_back();
    }

    void ScopeResolver::createNewNamespace(const std::string& name, bool with_prefix, bool is_glob, const std::vector<std::string>& symbols)
    {
        m_scopes.emplace_back(std::make_unique<NamespaceScope>(name, with_prefix, is_glob, symbols));
    }

    std::string ScopeResolver::registerInCurrent(const std::string& name, const bool is_mutable)
    {
        return m_scopes.back()->add(name, is_mutable);
    }

    void ScopeResolver::saveNamespaceAndRemove()
    {
        for (auto& m_scope : std::ranges::reverse_view(m_scopes) | std::ranges::views::drop(1))
        {
            if (m_scope->saveNamespace(m_scopes.back()))
                break;
        }

        m_scopes.pop_back();
    }

    std::optional<bool> ScopeResolver::isImmutable(const std::string& name) const
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe = m_scope->get(name, currentNamespace(), true); maybe.has_value())
                return !maybe.value().is_mutable;
        }
        return std::nullopt;
    }

    bool ScopeResolver::isRegistered(const std::string& name) const
    {
        const std::string origin_namespace = currentNamespace();
        return std::ranges::any_of(std::ranges::reverse_view(m_scopes), [&name, &origin_namespace](const auto& scope) {
            return scope->get(name, origin_namespace, true).has_value();
        });
    }

    bool ScopeResolver::isInScope(const std::string& name) const
    {
        return m_scopes.back()->get(name, currentNamespace(), false).has_value();
    }

    std::string ScopeResolver::getFullyQualifiedNameInNearestScope(const std::string& name) const
    {
        const std::string prefix = currentNamespace();
        std::optional<std::string> maybe_name;
        for (const auto& scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe_fqn = scope->get(name, prefix, true); maybe_fqn.has_value())
            {
                // prioritize non-hidden symbols
                if ((maybe_name.has_value() &&
                     maybe_name.value().ends_with("#hidden") &&
                     !maybe_fqn.value().name.ends_with("#hidden")) ||
                    !maybe_name.has_value())
                    maybe_name = maybe_fqn.value().name;
            }
        }
        return maybe_name.value_or(name);
    }

    std::pair<bool, std::string> ScopeResolver::canFullyQualifyName(const std::string& name)
    {
        // a given name can be fully qualified if
        // old == new
        // old != new and new has prefix
        //     if the prefix namespace is glob
        //     if the prefix namespace has name in its symbols
        //     if the prefix namespace is with_prefix && it is the top most scope
        const std::string maybe_fqn = getFullyQualifiedNameInNearestScope(name);

        if (maybe_fqn == name)
            return std::make_pair(true, maybe_fqn);

        const std::string prefix = maybe_fqn.substr(0, maybe_fqn.find_first_of(':'));
        const std::string unprefixed_name = name.substr(name.find_first_of(':') + 1);
        auto namespaces =
            std::ranges::reverse_view(m_scopes) | std::ranges::views::filter([](const auto& e) {
                return e->isNamespace();
            });
        bool top = true;
        for (auto& scope : namespaces)
        {
            if (top && prefix == scope->prefix())
                return std::make_pair(true, maybe_fqn);
            if (!top && prefix == scope->prefix() && (scope->isGlob() || scope->hasSymbol(name)))
                return std::make_pair(true, maybe_fqn);

            // check for the presence of the symbol in symbol imports and glob imports
            if (scope->recursiveHasSymbol(unprefixed_name))
                return std::make_pair(true, maybe_fqn);

            top = false;
        }

        return std::make_pair(false, maybe_fqn);
    }

    StaticScope* ScopeResolver::currentScope() const
    {
        if (!m_scopes.empty()) [[likely]]
            return m_scopes.back().get();
        return nullptr;
    }

    std::string ScopeResolver::currentNamespace() const
    {
        for (const auto& scope : std::ranges::reverse_view(m_scopes))
        {
            if (scope->isNamespace())
                return scope->prefix();
        }

        // no namespace name, thus no prefix ; "" is either the default namespace, a function scope or a while loop
        return "";
    }
}
