/**
 * @file NameResolutionPass.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief
 * @version 1.0
 * @date 2024-07-22
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTIONPASS_HPP
#define ARK_COMPILER_NAMERESOLUTIONPASS_HPP

#include <vector>
#include <string>
#include <optional>
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
struct std::hash<Ark::internal::Variable>
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
        /**
         * @brief Create a ScopeResolver
         * @details Kickstart by create a default global scope
         */
        ScopeResolver();

        /**
         * @brief Create a new scope
         */
        void createNew();

        /**
         * @brief Remove the last scope
         */
        void removeLocalScope();

        /**
         * @brief Register a variable in the current (last) scope
         * @param name
         * @param is_mutable
         */
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
         * @brief Checks if 'name' is in the current scope
         *
         * @param name
         * @return
         */
        [[nodiscard]] bool isInScope(const std::string& name) const;

        class Scope
        {
        public:
            /**
             * @brief Add a variable to the scope, given a mutability status
             * @param name
             * @param is_mutable
             */
            void add(const std::string& name, bool is_mutable);

            /**
             * @brief Try to return a variable from this scope with a given name.
             * @param name
             * @return std::optional<Variable> std::nullopt if the variable isn't in scope
             */
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
        /**
         * @brief Create a NameResolutionPass
         * @param debug debug level
         */
        explicit NameResolutionPass(unsigned debug);

        /**
         * @brief Start visiting the given AST, checking for mutability violation and unbound variables
         * @param ast AST to analyze
         */
        void process(const Node& ast) override;

        /**
         * @brief Unused overload that return the input AST (untouched as this pass only generates errors)
         * @return const Node& ast
         */
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
        std::unordered_set<std::string> m_language_symbols;  ///< Precomputed set of language symbols that can't be used to define variables
        std::vector<Node> m_symbol_nodes;
        std::unordered_set<std::string> m_defined_symbols;
        std::vector<std::string> m_plugin_names;
        ScopeResolver m_scope_resolver;

        /**
         * @brief Recursively visit nodes
         * @param node node to visit
         */
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
