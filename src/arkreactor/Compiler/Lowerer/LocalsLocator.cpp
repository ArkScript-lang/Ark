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
        auto& [data, type] = m_scopes.back();

        if (type != ScopeType::Closure)
        {
            // Compute the index of the variable in the active scope from the end.
            if (const auto it = std::ranges::find(data, name); it != data.end())
                return static_cast<std::size_t>(std::distance(it, data.end())) - 1;
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

    void LocalsLocator::saveScopeLengthForBranch()
    {
        m_drop_for_conds.push_back(m_scopes.back().data.size());
    }

    void LocalsLocator::dropVarsForBranch()
    {
        const auto old_length = m_drop_for_conds.back();
        m_drop_for_conds.pop_back();

        auto& back = m_scopes.back();
        if (back.data.size() > old_length)
            back.data.erase(
                back.data.begin() + static_cast<decltype(back.data)::difference_type>(old_length),
                back.data.end());
    }
}
