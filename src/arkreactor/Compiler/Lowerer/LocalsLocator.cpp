#include <Ark/Compiler/Lowerer/LocalsLocator.hpp>

#include <ranges>

namespace Ark::internal
{
    LocalsLocator::LocalsLocator()
    {
        // create a default scope
        m_scopes.emplace_back();
    }

    void LocalsLocator::addLocal(const std::string& name)
    {
        auto& scope = m_scopes.back();
        if (std::ranges::find(scope.data, name) == scope.data.end())
            scope.data.push_back(name);
    }

    std::optional<std::size_t> LocalsLocator::lookupLastScopeByName(const std::string& name)
    {
        auto& back = m_scopes.back();

        if (back.type != ScopeType::Closure)
        {
            // Compute the index of the variable in the active scope from the end.
            if (const auto it = std::ranges::find(back.data, name); it != back.data.end())
                return static_cast<std::size_t>(std::distance(it, back.data.end())) - 1;
        }

        return std::nullopt;
    }

    void LocalsLocator::createScope(const ScopeType type)
    {
        m_scopes.emplace_back(Scope {
            .data = {},
            .type = type });
    }

    void LocalsLocator::deleteScope()
    {
        m_scopes.pop_back();
    }
}
