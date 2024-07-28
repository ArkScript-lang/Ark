/**
 * @file Processor.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Handles the macros and their expansion in ArkScript source code
 * @version 2.0
 * @date 2021-02-18
 *
 * @copyright Copyright (c) 2021-2024
 *
 */

#ifndef COMPILER_MACROS_PROCESSOR_HPP
#define COMPILER_MACROS_PROCESSOR_HPP

#include <Ark/Compiler/AST/Node.hpp>
#include <Ark/Compiler/Macros/MacroScope.hpp>
#include <Ark/Compiler/Macros/Pipeline.hpp>

#include <unordered_map>
#include <string>

namespace Ark::internal
{
    /**
     * @brief The class handling the macros definitions and calls, given an AST
     *
     */
    class MacroProcessor
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
        void process(const Node& ast);

        /**
         * @brief Return the modified AST
         *
         * @return Node&
         */
        [[nodiscard]] const Node& ast() const noexcept;

        friend class MacroExecutor;

    private:
        unsigned m_debug;                  ///< The debug level
        Node m_ast;                        ///< The modified AST
        std::vector<MacroScope> m_macros;  ///< Handling macros in a scope fashion
        MacroExecutorPipeline m_executor_pipeline;
        std::vector<std::string> m_predefined_macros;  ///< Already existing macros, non-keywords, non-builtins
        std::unordered_map<std::string, Node> m_defined_functions;

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
         * @brief Check if a given symbol is a predefined macro or not
         *
         * @param symbol
         * @return true
         * @return false
         */
        bool isPredefined(const std::string& symbol);

        /**
         * @brief Recursively apply macros on a given node
         *
         * @param node
         */
        void recurApply(Node& node);

        /**
         * @brief Check if a given node is a list node, and starts with a Begin
         *
         * @param node
         * @return true if it starts with a Begin
         * @return false
         */
        static bool hadBegin(const Node& node);

        /**
         * @brief Remove a begin block added by a macro
         *
         * @param node
         * @param i
         */
        static void removeBegin(Node& node, std::size_t i);

        /**
         * @brief Check if a node can be evaluated at compile time
         *
         * @param node
         * @return true
         * @return false
         */
        [[nodiscard]] bool isConstEval(const Node& node) const;

        /**
         * @brief Registers macros based on their type
         * @details Validate macros and register them by their name
         *
         * @param node A node of type Macro
         */
        void registerMacro(Node& node);

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
         */
        void processNode(Node& node, unsigned depth);

        /**
         * @brief Apply a macro on a given node
         *
         * @param node
         * @param depth
         * @return true if a macro was applied
         * @return false
         */
        bool applyMacro(Node& node, unsigned depth) const;

        /**
         * @brief Unify a target node with a given map symbol => replacement node, recursively
         *
         * @param map
         * @param target
         * @param parent
         * @param index position of target inside parent->list()
         * @param unify_depth call depth to unify, to avoid deep recursive unification
         */
        void unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, std::size_t index, std::size_t unify_depth = 0);

        void setWithFileAttributes(const Node origin, Node& output, const Node& macro);

        /**
         * @brief Check if the given node has exactly the provided argument count, otherwise throws an error
         *
         * @param node a list node with a macro application, eg (= a b)
         * @param expected expected argument count, not counting the macro
         * @param name the name of the macro being applied
         * @param kind the macro kind, empty by default (eg "operator", "condition")
         */
        void checkMacroArgCount(const Node& node, std::size_t expected, const std::string& name, const std::string& kind = "");

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
        static bool isTruthy(const Node& node);

        /**
         * @brief Throw a macro processing error
         *
         * @param message the error
         * @param node the node in which there is an error
         */
        [[noreturn]] static void throwMacroProcessingError(const std::string& message, const Node& node);
    };
}

#endif
