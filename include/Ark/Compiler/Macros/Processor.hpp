/**
 * @file Processor.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Handles the macros and their expansion in ArkScript source code
 * @date 2021-02-18
 *
 * @copyright Copyright (c) 2021-2026
 *
 */

#ifndef COMPILER_MACROS_PROCESSOR_HPP
#define COMPILER_MACROS_PROCESSOR_HPP

#include <Ark/Utils/Platform.hpp>
#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/Macros/MacroScope.hpp>
#include <Ark/Compiler/Pass.hpp>

#include <unordered_map>
#include <optional>
#include <string>

namespace Ark::internal
{
    class MacroExecutor;

    /**
     * @brief The class handling the macros definitions and calls, given an AST
     *
     */
    class ARK_API MacroProcessor final : public Pass
    {
    public:
        /**
         * @brief Construct a new Macro Processor object
         *
         * @param debug the debug level
         */
        explicit MacroProcessor(unsigned debug) noexcept;

        /**
         * @brief Send the complete AST and work on it
         *
         * @param ast
         */
        void process(const Node& ast) override;

        /**
         * @brief Return the modified AST
         *
         * @return Node&
         */
        [[nodiscard]] const Node& ast() const noexcept override;

        friend class MacroExecutor;

    private:
        Node m_ast;                                ///< The modified AST
        std::vector<MacroScope> m_macros;          ///< Handling macros in a scope fashion
        std::vector<Node> m_macros_being_applied;  ///< Stack of macros being applied. The last one is the current one we are working on
        std::shared_ptr<MacroExecutor> m_conditional_executor;
        std::vector<std::shared_ptr<MacroExecutor>> m_executors;
        std::unordered_map<std::string, Node> m_defined_functions;

        /**
         * @brief Return std::nullopt if the function isn't registered, otherwise return its node
         *
         * @param name function name
         * @return std::optional<Node>
         */
        [[nodiscard]] std::optional<Node> lookupDefinedFunction(const std::string& name) const;

        /**
         * @brief Find the nearest macro matching a given name
         *
         * @param name
         * @return const Node* nullptr if no macro was found
         */
        [[nodiscard]] const Node* findNearestMacro(const std::string& name) const;

        /**
         * @brief Find the nearest macro matching a given name and delete it
         *
         * @param name
         */
        void deleteNearestMacro(const std::string& name);

        /**
         * @brief Check if a given node is a list node, and starts with a Begin
         *
         * @param node
         * @return true if it starts with a Begin
         * @return false
         */
        static bool isBeginNode(const Node& node);

        /**
         * @brief Remove a begin block added by a macro
         *
         * @param node
         * @param i
         */
        static void removeBegin(Node& node, std::size_t i);

        /**
         * @brief Registers macros based on their type, expand conditional macros
         * @details Validate macros and register them by their name
         *
         * @param node A node of type Macro
         * @param depth
         */
        void handleMacroNode(Node& node, unsigned depth);

        /**
         * @brief Registers a function definition node
         *
         * @param node
         */
        void registerFuncDef(const Node& node);

        /**
         * @brief Register macros in scopes and apply them as needed
         *
         * @param node node on which to operate
         * @param depth
         * @param is_processing_namespace
         */
        void processNode(Node& node, unsigned depth, bool is_processing_namespace = false);

        /**
         * @brief Apply a macro on a given node
         *
         * @param node
         * @param depth
         * @return true if a macro was applied
         * @return false
         */
        bool applyMacro(Node& node, unsigned depth);

        /**
         * @brief Check if the given node has exactly the provided argument count, otherwise throws an error
         *
         * @param node a list node with a macro application, eg (= a b)
         * @param expected expected argument count, not counting the macro
         * @param name the name of the macro being applied
         * @param is_expansion if the error message should switch from "Interpreting ..." to "When expanding ..."
         * @param kind the macro kind, empty by default (eg "operator", "condition")
         */
        void checkMacroArgCountEq(const Node& node, std::size_t expected, const std::string& name, bool is_expansion = false, const std::string& kind = "") const;

        /**
         * @brief Check if the given node has at least the provided argument count, otherwise throws an error
         *
         * @param node a list node with a macro application, eg (= a b)
         * @param expected expected argument count, not counting the macro
         * @param name the name of the macro being applied
         * @param kind the macro kind, empty by default (eg "operator", "condition")
         */
        void checkMacroArgCountGe(const Node& node, std::size_t expected, const std::string& name, const std::string& kind = "") const;

        /**
         * @brief Evaluate only the macros
         *
         * @param node
         * @param depth
         * @param is_not_body true if the method is run on a non-body code (eg a condition of an if-macro)
         * @return Node
         */
        Node evaluate(Node& node, unsigned depth, bool is_not_body = false);

        /**
         * @brief Check if a node can be evaluated to true
         *
         * @param node
         * @return true
         * @return false
         */
        [[nodiscard]] bool isTruthy(const Node& node) const;

        /**
         * @brief Throw a macro processing error
         *
         * @param message the error
         * @param node the node in which there is an error
         */
        [[noreturn]] void throwMacroProcessingError(const std::string& message, const Node& node) const;

        void checkMacroTypeError(const std::string& macro, const std::string& arg, NodeType expected, const Node& actual) const;
    };
}

#endif
