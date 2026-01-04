/**
 * @file ScopeResolver.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Handle scope resolution at compile time
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024-2026
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTION_SCOPERESOLVER_HPP
#define ARK_COMPILER_NAMERESOLUTION_SCOPERESOLVER_HPP

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <utility>

#include <Ark/Compiler/NameResolution/StaticScope.hpp>

namespace Ark::internal
{
    class ScopeResolver
    {
    public:
        /**
         * @brief Create a ScopeResolver
         * @details Kickstart by create a default global scope
         */
        ScopeResolver();

        ScopeResolver(const ScopeResolver&) = delete;
        ScopeResolver& operator=(const ScopeResolver&) = delete;
        ScopeResolver(ScopeResolver&&) = default;
        ScopeResolver& operator=(ScopeResolver&&) = default;

        /**
         * @brief Create a new scope
         */
        void createNew();

        /**
         * @brief Remove the last scope
         */
        void removeLastScope();

        /**
         * @brief Create a new namespace scope
         * @param name
         * @param with_prefix
         * @param is_glob
         * @param symbols
         */
        void createNewNamespace(const std::string& name, bool with_prefix, bool is_glob, const std::vector<std::string>& symbols);

        /**
         * @brief Register a Declaration in the current (last) scope
         * @param name
         * @param is_mutable
         * @return std::string the fully qualified name assigned by the scope
         */
        std::string registerInCurrent(const std::string& name, bool is_mutable);

        /**
         * @brief Save the last scope as a namespace, by attaching it to the nearest namespace scope
         * @details Also handle removing the scope from the scope pile.
         */
        void saveNamespaceAndRemove();

        /**
         * @brief Checks the scopes in reverse order for 'name' and returns its mutability status
         * @param name
         * @return std::nullopt if the Declaration could not be found
         * @return true if immutable
         * @return false if mutable
         */
        [[nodiscard]] std::optional<bool> isImmutable(const std::string& name) const;

        /**
         * @brief Checks if any scope has 'name', in reverse order
         * @param name
         * @return
         */
        [[nodiscard]] bool isRegistered(const std::string& name) const;

        /**
         * @brief Checks if 'name' is in the current scope
         *
         * @param name
         * @return
         */
        [[nodiscard]] bool isInScope(const std::string& name) const;

        /**
         * @brief Get a FQN from a variable name in the nearest scope it is declared in
         *
         * @param name
         * @return std::string
         */
        [[nodiscard]] std::string getFullyQualifiedNameInNearestScope(const std::string& name) const;

        /**
         * @brief Checks if a name can be fully qualified (allows only unprefixed names to be resolved by glob namespaces or inside their own namespace)
         *
         * @param name
         * @return std::pair<bool, std::string> if the name can be fully qualified, first element is true ; second element is the FQN
         */
        [[nodiscard]] std::pair<bool, std::string> canFullyQualifyName(const std::string& name);

        /**
         * @brief Return a non-owning raw pointer to the current scope
         *
         * @return StaticScope* non-owning pointer to the current scope
         * @return nullptr if there are no scope
         */
        [[nodiscard]] StaticScope* currentScope() const;

    private:
        std::vector<std::unique_ptr<StaticScope>> m_scopes;

        std::string currentNamespace() const;
    };
}

#endif  // ARK_COMPILER_NAMERESOLUTION_SCOPERESOLVER_HPP
