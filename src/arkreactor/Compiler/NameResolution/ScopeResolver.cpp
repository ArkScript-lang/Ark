#include <Ark/Compiler/NameResolution/ScopeResolver.hpp>

#include <ranges>

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
        m_scopes.back()->add(name, is_mutable);
        return m_scopes.back()->fullyQualifiedName(name);
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
            if (auto maybe = m_scope->get(name, true); maybe.has_value())
                return !maybe.value().is_mutable;
        }
        return std::nullopt;
    }

    bool ScopeResolver::isRegistered(const std::string& name) const
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            if (m_scope->get(name, true).has_value())
                return true;
        }
        return false;
    }

    bool ScopeResolver::isInScope(const std::string& name) const
    {
        return m_scopes.back()->get(name, false).has_value();
    }

    std::string ScopeResolver::getFullyQualifiedNameInNearestScope(const std::string& name) const
    {
        for (const auto& scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe_fqn = scope->get(name, true); maybe_fqn.has_value())
                return maybe_fqn.value().name;
        }
        return name;
    }

    std::pair<bool, std::string> ScopeResolver::canFullyQualifyName(const std::string& name)
    {
        // a given name can be fully qualified if
        // old == new
        // old != new and new has prefix
        //     if the prefix namespace is glob
        //     if the prefix namespace is with_prefix && it is the top most scope
        const std::string maybe_fqn = getFullyQualifiedNameInNearestScope(name);

        if (maybe_fqn == name)
            return std::make_pair(true, maybe_fqn);

        const std::string prefix = maybe_fqn.substr(0, maybe_fqn.find_first_of(':'));
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
}
