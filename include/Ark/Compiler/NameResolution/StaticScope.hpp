/**
 * @file StaticScope.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-11-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP
#define ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP

#include <string>
#include <optional>
#include <memory>
#include <unordered_set>

namespace Ark::internal
{
    struct Declaration
    {
        std::string name;
        bool is_mutable;

        bool operator==(const Declaration& other) const = default;
    };
}

template <>
struct std::hash<Ark::internal::Declaration>
{
    inline size_t operator()(const Ark::internal::Declaration& x) const noexcept
    {
        return std::hash<std::string> {}(x.name);
    }
};

namespace Ark::internal
{
    class StaticScope
    {
    public:
        virtual ~StaticScope() = default;

        /**
         * @brief Add a Declaration to the scope, given a mutability status
         * @param name
         * @param is_mutable
         */
        virtual void add(const std::string& name, bool is_mutable);

        /**
         * @brief Try to return a Declaration from this scope with a given name.
         * @param name
         * @return std::optional<Declaration> std::nullopt if the Declaration isn't in scope
         */
        [[nodiscard]] virtual std::optional<Declaration> get(const std::string& name) const;

        /**
         * @brief Given a Declaration name, compute its fully qualified name
         * @param name
         * @return std::string fully qualified name in the scope
         */
        [[nodiscard]] virtual std::string fullyQualifiedName(const std::string& name) const;

        /**
         * @brief Save an unprefixed namespace scope to help with lookup
         *
         * @return true if the scope was saved, on NamespaceScope
         * @return false on StaticScope
         */
        virtual bool saveUnprefixedNamespace(std::unique_ptr<StaticScope>&);

    private:
        std::unordered_set<Declaration> m_vars {};
    };

    class NamespaceScope final : public StaticScope
    {
    public:
        NamespaceScope(std::string name, bool with_prefix, bool is_glob, const std::vector<std::string>& symbols);

        /**
         * @brief Add a Declaration to the scope, given a mutability status
         * @param name
         * @param is_mutable
         */
        void add(const std::string& name, bool is_mutable) override;

        /**
         * @brief Try to return a Declaration from this scope with a given name.
         * @param name
         * @return std::optional<Declaration> std::nullopt if the Declaration isn't in scope
         */
        [[nodiscard]] std::optional<Declaration> get(const std::string& name) const override;

        /**
         * @brief Given a Declaration name, compute its fully qualified name
         * @param name
         * @return std::string fully qualified name in the namespace
         */
        [[nodiscard]] std::string fullyQualifiedName(const std::string& name) const override;

        /**
         * @brief Save an unprefixed namespace scope to help with lookup
         *
         * @return true if the scope was saved, on NamespaceScope
         * @return false on StaticScope
         */
        bool saveUnprefixedNamespace(std::unique_ptr<StaticScope>&) override;

    private:
        std::string m_namespace;
        bool m_with_prefix;
        bool m_is_glob;
        std::vector<std::string> m_symbols;
        std::unordered_set<Declaration> m_vars {};
        std::vector<std::unique_ptr<StaticScope>> m_unprefixed_namespaces;
    };
}

#endif  // ARK_COMPILER_NAMERESOLUTION_STATICSCOPE_HPP
