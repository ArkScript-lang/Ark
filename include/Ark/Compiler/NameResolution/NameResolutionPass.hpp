/**
 * @file NameResolutionPass.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Resolves names and fully qualify them in the AST (prefixing them with the package they are from)
 * @date 2024-07-22
 *
 * @copyright Copyright (c) 2024-2025
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTIONPASS_HPP
#define ARK_COMPILER_NAMERESOLUTIONPASS_HPP

#include <vector>
#include <string>
#include <unordered_set>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Compiler/NameResolution/ScopeResolver.hpp>

namespace Ark::internal
{
    class ARK_API NameResolutionPass final : public Pass
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
        std::string addDefinedSymbol(const std::string& sym, bool is_mutable);

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
         * @param register_declarations whether or not the visit should register declarations
         */
        void visit(Node& node, bool register_declarations);

        /**
         *
         * @param node a list node whose first child is a keyword
         * @param keyword
         * @param register_declarations whether or not the visit should register declarations
         */
        void visitKeyword(Node& node, Keyword keyword, bool register_declarations);

        /**
         * @brief Register a given node in the symbol table
         *
         * @param symbol
         * @param old_name old name for symbol, to replace it with the new one if it was renamed
         */
        void addSymbolNode(const Node& symbol, const std::string& old_name = "");

        /**
         * @brief Checking if a symbol may be coming from a plugin
         *
         * @param name symbol name
         * @return true the symbol may be from a plugin, loaded at runtime
         * @return false
         */
        [[nodiscard]] bool mayBeFromPlugin(const std::string& name) const noexcept;

        std::string updateSymbolWithFullyQualifiedName(Node& symbol);

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
