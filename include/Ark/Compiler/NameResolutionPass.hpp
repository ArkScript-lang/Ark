/**
 * @file NameResolutionPass.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-07-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTIONPASS_HPP
#define ARK_COMPILER_NAMERESOLUTIONPASS_HPP

#include <vector>
#include <string>
#include <unordered_set>

#include <Ark/Compiler/Pass.hpp>

namespace Ark::internal
{
    struct Variable
    {
        std::string name;
        bool is_mutable;

        bool operator==(const Variable& other) const = default;
    };
}

template <>
struct ::std::hash<Ark::internal::Variable>
{
    inline size_t operator()(const Ark::internal::Variable& x) const noexcept
    {
        return std::hash<std::string> {}(x.name);
    }
};

namespace Ark::internal
{
    class ScopeResolver
    {
    public:
        ScopeResolver();

        void createNew();

        void removeLocalScope();

        void registerInCurrent(const std::string& name, bool is_mutable);

        /**
         * @brief Checks the scopes in reverse order for 'name' and returns its mutability status
         * @param name
         * @return std::nullopt if the variable could not be found
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
         * @brief Returns false if we have only one scope (global), checks the scopes recursively for 'name'
         *
         * @param name
         * @return
         */
        [[nodiscard]] bool isLocalVar(const std::string& name) const;

        /**
         * @brief 'name' has to be defined only in the first scope to be considered global
         *
         * @param name
         * @return
         */
        [[nodiscard]] bool isGlobalVar(const std::string& name) const;

        /**
         * @brief Checks if 'name' is in the current scope
         *
         * @param name
         * @return
         */
        [[nodiscard]] bool isInScope(const std::string& name) const;

        class Scope
        {
        public:
            void add(const std::string& name, bool is_mutable);

            [[nodiscard]] std::optional<Variable> get(const std::string& name) const;

            [[nodiscard]] bool has(const std::string& name) const;

        private:
            std::unordered_set<Variable> m_vars {};
        };

    private:
        std::vector<Scope> m_scopes;
    };

    class NameResolutionPass final : public Pass
    {
    public:
        explicit NameResolutionPass(unsigned debug);

        void process(const Node& ast) override;

        [[nodiscard]] const Node& ast() const noexcept override;

        /**
         * @brief Register a symbol as defined, so that later we can throw errors on undefined symbols
         *
         * @param sym
         * @param is_mutable true if the symbol is inside mut/set, false otherwise (let)
         */
        void addDefinedSymbol(const std::string& sym, bool is_mutable);

    private:
        Node m_ast;
        std::unordered_set<std::string> m_language_symbols;
        std::vector<Node> m_symbol_nodes;
        std::unordered_set<std::string> m_defined_symbols;
        std::vector<std::string> m_plugin_names;
        ScopeResolver m_scope_resolver;

        void visit(const Node& node);

        /**
         *
         * @param node a list node whose first child is a keyword
         * @param keyword
         */
        void visitKeyword(const Node& node, Keyword keyword);

        /**
         * @brief Register a given node in the symbol table
         *
         * @param symbol
         */
        void addSymbolNode(const Node& symbol);

        /**
         * @brief Checking if a symbol may be coming from a plugin
         *
         * @param name symbol name
         * @return true the symbol may be from a plugin, loaded at runtime
         * @return false
         */
        [[nodiscard]] bool mayBeFromPlugin(const std::string& name) const noexcept;

        /**
         * @brief Checks for undefined symbols, not present in the defined symbols table
         *
         */
        void checkForUndefinedSymbol() const;

        /**
         * @brief Suggest a symbol of what the user may have meant to input
         *
         * @param str the string
         * @return std::string
         */
        [[nodiscard]] std::string offerSuggestion(const std::string& str) const;
    };
}

#endif  // ARK_COMPILER_NAMERESOLUTIONPASS_HPP
