/**
 * @file MacroProcessor.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Handles the macros and their expansion in ArkScript source code
 * @version 0.1
 * @date 2021-02-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ARK_COMPILER_MACROPROCESSOR_HPP
#define ARK_COMPILER_MACROPROCESSOR_HPP

#include <Ark/Compiler/Node.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Compiler/makeNodeBasedError.hpp>

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

    private:
        unsigned m_debug;  ///< The debug level
        uint16_t m_options;
        Node m_ast;  ///< The modified AST
        std::vector<std::unordered_map<std::string, Node>> m_macros;  ///< Handling macros in a scope fashion

        Node m_trueNode, m_falseNode, m_nilNode;

        /**
         * @brief Find the nearest macro matching a giving name
         * 
         * @param name 
         * @return Node* nullptr if no macro was found
         */
        inline Node* find_nearest_macro(const std::string& name)
        {
            if (m_macros.empty())
                return nullptr;

            for (auto it = m_macros.rbegin(); it != m_macros.rend(); ++it)
            {
                if (it->size() != 0)
                {
                    auto res = it->find(name);
                    if (res != it->end())
                        return &res->second;
                }
            }
            return nullptr;
        }

        /**
         * @brief Registers macros based on their type
         * @details Validate macros and register them by their name
         * 
         * @param node A node of type Macro
         */
        void registerMacro(Node& node);

        /**
         * @brief Register macros in scopes and apply them as needed
         * 
         * @param node node on which to operate
         * @param depth
         */
        void process(Node& node, unsigned depth);

        /**
         * @brief Execute a node, trying to emplace macros calls
         * 
         * @param node 
         */
        void execute(Node& node);

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
}

#endif
