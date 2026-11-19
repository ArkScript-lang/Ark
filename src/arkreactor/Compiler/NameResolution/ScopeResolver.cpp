#include <Ark/Compiler/NameResolution/ScopeResolver.hpp>

#include <queue>
#include <ranges>
#include <algorithm>

#include <Ark/Compiler/Common.hpp>

namespace Ark::internal
{
    void ScopeResolver::createNew()
    {
        m_scopes.emplace_back(std::make_unique<StaticScope>());
    }

    void ScopeResolver::removeLastScope()
    {
        m_scopes.pop_back();
    }

    void ScopeResolver::createNewNamespace(const std::string& prefix, const std::string& package, const std::vector<Import>& imports)
    {
        m_scopes.emplace_back(std::make_unique<NamespaceScope>(prefix, package, imports));
    }

    std::string ScopeResolver::registerInCurrent(const std::string& name, const bool is_mutable)
    {
        return m_scopes.back()->add(name, is_mutable);
    }

    void ScopeResolver::saveNamespaceAndRemove()
    {
        m_saved_scopes.emplace_back(std::move(m_scopes.back()));
        m_scopes.pop_back();
    }

    std::optional<bool> ScopeResolver::isImmutable(const std::string& name) const
    {
        for (const auto& m_scope : std::ranges::reverse_view(m_scopes))
        {
            // todo: bfs lookup?
            if (auto maybe = m_scope->get(name, currentNamespace()->prefix()); maybe.has_value())
                return !maybe.value().is_mutable;
        }
        return std::nullopt;
    }

    bool ScopeResolver::isRegistered(const std::string& name) const
    {
        const std::string origin_namespace = currentNamespace()->prefix();
        return std::ranges::any_of(std::ranges::reverse_view(m_scopes), [&name, &origin_namespace](const auto& scope) {
            // todo: bfs lookup?
            return scope->get(name, origin_namespace).has_value();
        });
    }

    bool ScopeResolver::isInScope(const std::string& name) const
    {
        return m_scopes.back()->get(name, currentNamespace()->prefix()).has_value();
    }

    struct ScopeAndOrigin
    {
        Import imported_as;
        StaticScope* scope;
    };

    std::string ScopeResolver::getFullyQualifiedNameInNearestScope(const std::string& name) const
    {
        const StaticScope* ns = currentNamespace();
        const std::string prefix = ns->prefix();
        const std::string name_prefix = name.substr(0, name.find_first_of(':'));
        std::optional<std::string> maybe_name;

        for (const auto& scope : std::ranges::reverse_view(m_scopes))
        {
            if (auto maybe_fqn = scope->get(name, prefix); maybe_fqn.has_value())
            {
                if (!maybe_name.has_value())
                    maybe_name = maybe_fqn->name;
            }
        }

        // if we didn't find a FQN for 'name' in the pile of active scopes, try via the import list
        // todo: put this in a separate method
        if (!maybe_name)
        {
            std::queue<ScopeAndOrigin> queue;
            // todo: O(n^2) ; can we do better?
            for (const auto& candidate : ns->imports())
            {
                auto it = std::ranges::find_if(
                    m_saved_scopes,
                    [&candidate](const std::unique_ptr<StaticScope>& scope) -> bool {
                        return scope->package() == candidate.package;
                    });
                if (it != m_saved_scopes.end())
                    queue.push(ScopeAndOrigin { candidate, it->get() });
            }

            while (!queue.empty())
            {
                const auto& [imported_as, scope] = queue.front();

                if (auto maybe_fqn = scope->get(name, prefix); maybe_fqn.has_value())
                {
                    if ((imported_as.with_prefix && imported_as.prefix == name_prefix) ||
                        imported_as.is_glob ||
                        std::ranges::find(imported_as.symbols, name) != imported_as.symbols.end())
                    {
                        maybe_name = maybe_fqn->name;
                        break;
                    }
                }
                else
                {
                    // todo: perf wise, it sucks
                    for (const auto& candidate : scope->imports())
                    {
                        auto it = std::ranges::find_if(
                            m_saved_scopes,
                            [&candidate](const std::unique_ptr<StaticScope>& s) -> bool {
                                return s->package() == candidate.package;
                            });
                        if (it != m_saved_scopes.end())
                            queue.push(ScopeAndOrigin { candidate, it->get() });
                    }
                }

                queue.pop();
            }
        }

        return maybe_name.value_or("");
    }

    std::pair<std::optional<QualifyingFailReason>, std::string> ScopeResolver::tryFullyQualifyName(const std::string& name)
    {
        // search in current scope and parent static scopes
        // if nothing, look in other scopes that are in the namespace imports (direct + indirect)
        // filter using the import list (glob, symbols, or prefix)
        const std::string maybe_fqn = getFullyQualifiedNameInNearestScope(name);

        if (maybe_fqn.empty())
            // todo
            return std::make_pair(QualifyingFailReason::NeedPrefixing, name);

        // const std::string prefix = maybe_fqn.substr(0, maybe_fqn.find_first_of(':'));
        // const std::string unprefixed_name = name.substr(name.find_first_of(':') + 1);

        return std::make_pair(std::nullopt, maybe_fqn);
    }

    StaticScope* ScopeResolver::currentScope() const
    {
        if (!m_scopes.empty()) [[likely]]
            return m_scopes.back().get();
        return nullptr;
    }

    StaticScope* ScopeResolver::currentNamespace() const
    {
        for (const auto& scope : std::ranges::reverse_view(m_scopes))
        {
            if (scope->isNamespace())
                return scope.get();
        }

        // this shouldn't reach, since we MUST have a namespace scope as the first created scope
        // but just in case, let's have a default value
        return m_scopes.front().get();
    }
}
