/**
 * @file StaticScope.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Static scopes (for functions, loops) and namespace scopes (for packages) definitions, used at compile time
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024-2025
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP
#define ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP

#include <string>
#include <optional>
#include <memory>
#include <vector>
#include <ranges>
#include <unordered_set>

#include <Ark/Utils/Platform.hpp>

namespace Ark::internal
{
    struct Declaration
    {
        std::string name;           ///< End name, can be modified to be hidden
        std::string original_name;  ///< Original name, with the prefix, without hidden namespaces
        bool is_mutable;

        bool operator==(const Declaration& other) const = default;
    };
}

template <>
struct std::hash<Ark::internal::Declaration>
{
    inline size_t operator()(const Ark::internal::Declaration& x) const noexcept
    {
        return std::hash<std::string> {}(x.original_name);
    }
};

namespace Ark::internal
{
    class ARK_API StaticScope
    {
    public:
        virtual ~StaticScope() = default;

        StaticScope() = default;

        StaticScope(const StaticScope&) = delete;
        StaticScope& operator=(const StaticScope&) = delete;
        StaticScope(StaticScope&&) = default;
        StaticScope& operator=(StaticScope&&) = default;

        /**
         * @brief Add a Declaration to the scope, given a mutability status
         * @param name
         * @param is_mutable
         */
        virtual std::string add(const std::string& name, bool is_mutable);

        /**
         * @brief Try to return a Declaration from this scope with a given name.
         * @param name
         * @param extensive_lookup unused in StaticScope
         * @return std::optional<Declaration> std::nullopt if the Declaration isn't in scope
         */
        [[nodiscard]] virtual std::optional<Declaration> get(const std::string& name, bool extensive_lookup);

        /**
         * @brief Given a Declaration name, compute its fully qualified name
         * @param name
         * @return std::string fully qualified name in the scope
         */
        [[nodiscard]] virtual std::string fullyQualifiedName(const std::string& name) const;

        /**
         * @brief Save a namespace scope to help with lookup
         *
         * @return true if the scope was saved, on NamespaceScope
         * @return false on StaticScope
         */
        virtual bool saveNamespace(std::unique_ptr<StaticScope>&);

        [[nodiscard]] virtual bool isNamespace() const;
        [[nodiscard]] inline virtual bool isGlob() const { return false; }
        [[nodiscard]] inline virtual std::string prefix() const { return ""; }
        [[nodiscard]] inline virtual bool hasSymbol(const std::string&) const { return false; }
        [[nodiscard]] inline virtual bool recursiveHasSymbol(const std::string&) { return false; }

    private:
        std::unordered_set<Declaration> m_vars {};
    };

    class ARK_API NamespaceScope final : public StaticScope
    {
    public:
        NamespaceScope(std::string name, bool with_prefix, bool is_glob, const std::vector<std::string>& symbols);

        NamespaceScope(const NamespaceScope&) = delete;
        NamespaceScope& operator=(const NamespaceScope&) = delete;
        NamespaceScope(NamespaceScope&&) = default;
        NamespaceScope& operator=(NamespaceScope&&) = default;

        /**
         * @brief Add a Declaration to the scope, given a mutability status
         * @param name
         * @param is_mutable
         */
        std::string add(const std::string& name, bool is_mutable) override;

        /**
         * @brief Try to return a Declaration from this scope with a given name.
         * @param name
         * @param extensive_lookup if true, use the additional saved namespaces
         * @return std::optional<Declaration> std::nullopt if the Declaration isn't in scope
         */
        [[nodiscard]] std::optional<Declaration> get(const std::string& name, bool extensive_lookup) override;

        /**
         * @brief Given a Declaration name, compute its fully qualified name
         * @param name
         * @return std::string fully qualified name in the namespace
         */
        [[nodiscard]] std::string fullyQualifiedName(const std::string& name) const override;

        /**
         * @brief Save a namespace scope to help with lookup
         *
         * @return true if the scope was saved, on NamespaceScope
         * @return false on StaticScope
         */
        bool saveNamespace(std::unique_ptr<StaticScope>&) override;

        [[nodiscard]] bool isNamespace() const override;
        [[nodiscard]] inline bool isGlob() const override { return m_is_glob; }
        [[nodiscard]] inline std::string prefix() const override { return m_namespace; }
        [[nodiscard]] inline bool hasSymbol(const std::string& symbol) const override { return std::ranges::find(m_symbols, symbol) != m_symbols.end(); }
        [[nodiscard]] bool recursiveHasSymbol(const std::string& symbol) override;

    private:
        std::string m_namespace;
        bool m_with_prefix;
        bool m_is_glob;
        std::vector<std::string> m_symbols;
        std::unordered_set<Declaration> m_vars {};
        std::vector<std::unique_ptr<StaticScope>> m_additional_namespaces;
    };
}

#endif  // ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP
