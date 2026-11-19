/**
 * @file NameResolutionPass.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Resolves names and fully qualify them in the AST (prefixing them with the package they are from)
 * @date 2024-07-22
 *
 * @copyright Copyright (c) 2024-2026
 *
 */

#ifndef ARK_COMPILER_NAMERESOLUTIONPASS_HPP
#define ARK_COMPILER_NAMERESOLUTIONPASS_HPP

#include <vector>
#include <string>
#include <unordered_set>

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/Pass.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/NameResolution/ScopeResolver.hpp>

namespace Ark::internal
{
    class ARK_API NameResolutionPass final : public Pass
    {
    public:
        /**
         * @brief Create a NameResolutionPass
         *
         * @param debug debug level
         * @param stats_collector optional statistics collector
         */
        explicit NameResolutionPass(unsigned debug, Statistics* stats_collector = nullptr);

        /**
         * @brief Configure the NameResolutionPass with the import list of the root package
         *
         * @param root_import_list
         */
        void setup(const std::vector<Import>& root_import_list);

        /**
         * @brief Start visiting the given AST, checking for mutability violation and unbound variables
         *
         * @param ast AST to analyse
         */
        void process(const Node& ast);

        /**
         * @brief Unused overload that return the input AST (untouched as this pass only generates errors)
         *
         * @return const Node& ast
         */
        [[nodiscard]] const Node& ast() const noexcept;

        /**
         * @brief Register a symbol as defined, so that later we can throw errors on undefined symbols
         *
         * @param sym
         * @param is_mutable true if the symbol is inside mut/set, false otherwise (let)
         */
        std::string qualifyAndRegisterSymbol(const std::string& sym, bool is_mutable);

    private:
        Node m_ast;
        std::unordered_set<std::string> m_language_symbols;  ///< Precomputed set of language symbols that can't be used to define variables
        std::vector<Node> m_symbol_nodes;
        std::unordered_set<std::string> m_defined_symbols;
        std::vector<std::string> m_plugin_names;
        ScopeResolver m_scope_resolver;

        /**
         * @brief Recursively visit nodes and register variable declarations
         *
         * @param node node to visit
         */
        void registerDeclarations(Node& node);

        /**
         * @brief Recursively visit nodes and updates symbol names
         *
         * @param node node to visit and update
         */
        void qualifyNames(Node& node);

        /**
         * @brief Register a given node in the symbol table
         *
         * @param symbol
         */
        void savedAsKnownSymbol(const Node& symbol);

        /**
         * @brief Checking if a symbol may be coming from a plugin
         *
         * @param name symbol name
         * @return true the symbol may be from a plugin, loaded at runtime
         * @return false
         */
        [[nodiscard]] bool mayBeFromPlugin(const std::string& name) const noexcept;

        std::string tryQualifyAndUpdateSymbol(Node& symbol);

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
