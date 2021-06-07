/**
 * @file MacroProcessor.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Handles the macros and their expansion in ArkScript source code
 * @version 0.3
 * @date 2021-02-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_MACROPROCESSOR_HPP
#define ARK_COMPILER_MACROPROCESSOR_HPP

#include <Ark/Compiler/Node.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Compiler/makeErrorCtx.hpp>
#include <Ark/Compiler/MacroExecutor.hpp>
#include <Ark/Compiler/MacroExecutors/MacroExecutorPipeline.hpp>

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <string>
#include <cinttypes>

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
         * @param options the options flags
         */
        MacroProcessor(unsigned debug, uint16_t options) noexcept;

        /**
         * @brief Send the complete AST (after the inclusions and stuff), and work on it
         * 
         * @param ast 
         */
        void feed(const Node& ast);

        /**
         * @brief Return the modified AST
         * 
         * @return Node& 
         */
        const Node& ast() const noexcept;

        friend class MacroExecutor;

    private:
        unsigned m_debug;  ///< The debug level
        uint16_t m_options;
        Node m_ast;  ///< The modified AST
        std::vector<std::unordered_map<std::string, Node>> m_macros;  ///< Handling macros in a scope fashion
        std::unique_ptr<MacroExecutorPipeline> m_executor_pipeline;
        std::vector<std::string> m_predefined_macros;  ///< Already existing macros, non-keywords, non-builtins
        std::unordered_map<std::string, Node> m_defined_functions;

        /**
         * @brief Find the nearest macro matching a given name
         * 
         * @param name 
         * @return Node* nullptr if no macro was found
         */
        inline Node* findNearestMacro(const std::string& name);

        /**
         * @brief Find the nearest macro matching a given name and delete it
         * 
         * @param name 
         */
        inline void deleteNearestMacro(const std::string& name);

        /**
         * @brief Check if a given symbol is a predefined macro or not
         * 
         * @param symbol 
         * @return true 
         * @return false 
         */
        inline bool isPredefined(const std::string& symbol);

        /**
         * @brief Recursively apply macros on a given node
         * 
         * @param node 
         */
        inline void recurApply(Node& node);

        /**
         * @brief Check if a given node is a list node, and starts with a Begin
         * 
         * @param node 
         * @return true if it starts with a Begin
         * @return false 
         */
        inline bool hadBegin(const Node& node);

        /**
         * @brief Remove a begin block added by a macro
         * 
         * @param node 
         * @param i 
         */
        inline void removeBegin(Node& node, std::size_t& i);

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
        void registerFuncDef(Node& node);

        /**
         * @brief Register macros in scopes and apply them as needed
         * 
         * @param node node on which to operate
         * @param depth
         */
        void process(Node& node, unsigned depth);

        /**
         * @brief Apply a macro on a given node
         * 
         * @param node 
         * @return true if a macro was applied
         * @return false 
         */
        bool applyMacro(Node& node);

        /**
         * @brief Unify a target node with a given map symbol => replacement node, recursively
         * 
         * @param map 
         * @param target 
         * @param parent 
         * @param index position of target inside parent->list()
         */
        void unify(const std::unordered_map<std::string, Node>& map, Node& target, Node* parent, std::size_t index=0);

        /**
         * @brief Evaluate only the macros
         * 
         * @param node 
         * @param is_not_body true if the method is run on a non-body code (eg a condition of an if-macro)
         * @return Node 
         */
        Node evaluate(Node& node, bool is_not_body=false);

        /**
         * @brief Check if a node can be evaluated to true
         * 
         * @param node 
         * @return true 
         * @return false 
         */
        bool isTruthy(const Node& node);

        /**
         * @brief Throw a macro processing error
         * 
         * @param message the error
         * @param node the node in which there is an error
         */
        inline void throwMacroProcessingError(const std::string& message, const Node& node)
        {
            throw MacroProcessingError(makeNodeBasedErrorCtx(message, node));
        }
    };

    #include "inline/MacroProcessor.inl"
}

#endif
