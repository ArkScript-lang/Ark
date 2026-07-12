/**
 * @file LocalsLocator.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Track locals at compile
 * @date 2025-03-20
 *
 * @copyright Copyright (c) 2025-2026
 *
 */

#ifndef ARK_COMPILER_LOWERER_LOCALSLOCATOR_HPP
#define ARK_COMPILER_LOWERER_LOCALSLOCATOR_HPP

#include <vector>
#include <string>
#include <optional>

namespace Ark::internal
{
    class LocalsLocator
    {
    public:
        enum class ScopeType
        {
            Default,
            Function,
            Closure
        };

        /**
         * @brief Create a new LocalsLocator to track the position of variables in the scope stack
         */
        LocalsLocator();

        /**
         * @brief Register a local in the current scope, triggered by a STORE instruction. If the local already exists, it won't be added.
         *
         * @param name local's name
         */
        void addLocal(const std::string& name);

        /**
         * @brief Search for a local in the current scope. Returns std::nullopt in case of closure scopes or if the variable is in a parent scope.
         *
         * @param name local's name
         * @return std::optional<std::size_t>
         */
        std::optional<std::size_t> lookupLastScopeByName(const std::string& name);

        /**
         * @brief Create a new scope
         *
         * @param type scope type, default `ScopeType::Default`
         */
        void createScope(ScopeType type = ScopeType::Default);

        /**
         * @brief Delete the last scope
         */
        void deleteScope();

        /**
         * @brief Save the current scope length before entering a branch, so that we can ignore variable definitions inside the branch and generate valid indices
         */
        void saveScopeLengthForBranch();

        /**
         * @brief Drop potentially defined variables in the last saved branch
         *
         * @return true if vars were dropped, meaning that we created at least one variable in a branch
         * @return false if nothing was dropped
         */
        [[nodiscard]] bool dropVarsForBranch();

        /**
         * @brief Mark the last variable of a scope as unreachable, blocking lookupLastScopeByName(..) from returning a value past that point
         */
        void markLastLocalAsUnreachable();

    private:
        struct Scope
        {
            struct Var
            {
                std::string name;
                bool unreachable = false;
            };

            std::vector<Var> data;
            ScopeType type;
        };

        std::vector<Scope> m_scopes;
        std::vector<std::size_t> m_drop_for_conds;  ///< Needed to drop variables inside if/else branches since they don't have their own scope
    };
}

#endif  // ARK_COMPILER_LOWERER_LOCALSLOCATOR_HPP
