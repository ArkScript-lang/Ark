#include <Ark/Compiler/NameResolution/ScopeResolver.hpp>

#include <ranges>

namespace Ark::internal
{
    ScopeResolver::ScopeResolver()
    {
        createNewNamespace("", /* with_prefix= */ false, /* is_glob= */ true, /* symbols= */ {});
    }

    StaticScope* ScopeResolver::createNew()
    {
        return m_scopes.emplace_back(std::make_unique<StaticScope>()).get();
    }

    void ScopeResolver::removeLastScope()
    {
        m_scopes.pop_back();
    }

    NamespaceScope* ScopeResolver::createNewNamespace(const std::string& name, bool with_prefix, bool is_glob, const std::vector<std::string>& symbols)
    {
        StaticScope* scope = m_scopes.emplace_back(std::make_unique<NamespaceScope>(name, with_prefix, is_glob, symbols)).get();
        return dynamic_cast<NamespaceScope*>(scope);
    }

    std::string ScopeResolver::registerInCurrent(const std::string& name, const bool is_mutable)
    {
        m_scopes.back()->add(name, is_mutable);
        return m_scopes.back()->fullyQualifiedName(name);
    }

    void ScopeResolver::saveUnprefixedNamespaceAndRemove()
    {
        for (auto& m_scope : std::ranges::reverse_view(m_scopes) | std::ranges::views::drop(1))
        {
            if (m_scope->saveUnprefixedNamespace(m_scopes.back()))
                break;
        }

        m_scopes.pop_back();
    }

    std::optional<bool> ScopeResolver::isImmutable(const std::string& name) const
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe = m_scope->get(name); maybe.has_value())
                return !maybe.value().is_mutable;
        }
        return std::nullopt;
    }

    bool ScopeResolver::isRegistered(const std::string& name) const
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            if (m_scope->get(name).has_value())
                return true;
        }
        return false;
    }

    bool ScopeResolver::isInScope(const std::string& name) const
    {
        return m_scopes.back()->get(name).has_value();
    }

    std::string ScopeResolver::getFullyQualifiedNameInNearestScope(const std::string& name)
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe_fqn = m_scope->get(name); maybe_fqn.has_value())
                return maybe_fqn.value().name;
        }
        return name;
    }

    StaticScope* ScopeResolver::currentScope() const
    {
        if (!m_scopes.empty()) [[likely]]
            return m_scopes.back().get();
        return nullptr;
    }
}
