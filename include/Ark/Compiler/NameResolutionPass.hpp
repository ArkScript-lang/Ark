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

#include <Ark/Compiler/Pass.hpp>

namespace Ark::internal
{
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
         */
        void addDefinedSymbol(const std::string& sym);

    private:
        Node m_ast;
        // fixme: use a map/unordered_map/set/unordered_set for faster lookups?
        //        we can pre compute the operators+builtins+list operators
        std::vector<Node> m_symbol_nodes;
        std::vector<std::string> m_defined_symbols;
        std::vector<std::string> m_plugin_names;

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
