/**
 * @file StaticScope.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Static scopes (for functions, loops) and namespace scopes (for packages) definitions, used at compile time
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024-2026
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP
#define ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP

#include <string>
#include <optional>
#include <vector>
#include <unordered_set>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/AST/Import.hpp>

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
         * @param origin_namespace unused in StaticScope
         * @return std::optional<Declaration> std::nullopt if the Declaration isn't in scope
         */
        [[nodiscard]] virtual std::optional<Declaration> get(const std::string& name, const std::string& origin_namespace);

        /**
         * @brief Given a Declaration name, compute its fully qualified name
         * @param name
         * @return std::string fully qualified name in the scope
         */
        [[nodiscard]] virtual std::string fullyQualifiedName(const std::string& name) const;

        [[nodiscard]] virtual bool isNamespace() const;
        [[nodiscard]] virtual std::string prefix() const { return ""; }
        [[nodiscard]] virtual std::string package() const { return ""; }
        [[nodiscard]] virtual std::vector<Import> imports() const { return {}; }

    private:
        std::unordered_set<Declaration> m_vars {};
    };

    class ARK_API NamespaceScope final : public StaticScope
    {
    public:
        NamespaceScope(std::string prefix, std::string package, const std::vector<Import>& imports);

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
         * @param origin_namespace namespace's name of the variable we are trying to find
         * @return std::optional<Declaration> std::nullopt if the Declaration isn't in scope
         */
        [[nodiscard]] std::optional<Declaration> get(const std::string& name, const std::string& origin_namespace) override;

        /**
         * @brief Given a Declaration name, compute its fully qualified name
         * @param name
         * @return std::string fully qualified name in the namespace
         */
        [[nodiscard]] std::string fullyQualifiedName(const std::string& name) const override;

        [[nodiscard]] bool isNamespace() const override;
        [[nodiscard]] std::string prefix() const override { return m_namespace; }
        [[nodiscard]] std::string package() const override { return m_package; }
        [[nodiscard]] std::vector<Import> imports() const override { return m_imports; }

    private:
        std::string m_namespace;  ///< Prefix for the namespace, eg egg
        std::string m_package;  ///< Complete package name, eg foo.bar.egg
        std::vector<Import> m_imports;
        std::unordered_set<Declaration> m_vars {};
    };
}

#endif  // ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP
