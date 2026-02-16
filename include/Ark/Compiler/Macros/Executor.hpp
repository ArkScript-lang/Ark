/**
 * @file Executor.hpp
 * @author Ray John Alovera (rakista112@gmail.com), Lex Plateau (lexplt.dev@gmail.com)
 * @brief The base class for all MacroExecutors
 * @date 2024-03-03
 *
 * @copyright Copyright (c) 2021-2026
 *
 */

#ifndef ARK_COMPILER_EXECUTOR_HPP
#define ARK_COMPILER_EXECUTOR_HPP

#include <optional>
#include <unordered_map>

#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    class MacroProcessor;

    /**
     * @brief A class that applies macros in a Node
     *
     */
    class MacroExecutor
    {
    public:
        /**
         * @brief Construct a new Macro Executor object
         *
         * @param processor
         * @param debug
         */
        explicit MacroExecutor(MacroProcessor* processor, unsigned debug = 0);

        /**
         * @brief Need a virtual destructor to correctly destroy object.
         *
         */
        virtual ~MacroExecutor() = default;

        /**
         * @brief Executes macros in the Node if the Executor can handle it
         *
         * @param node the node that contains a macro
         * @param depth
         * @return true if a macro was successfully applied
         * @return false
         */
        virtual bool applyMacro(Node& node, unsigned depth) = 0;

        /**
         * @brief Checks if the executor can apply a macro on the passed Node
         *
         * @param node the node that contains a macro
         */
        virtual bool canHandle(Node& node) = 0;

        /**
         * @brief Returns the macro node that will be expanded
         *
         * @param node AST node on which the executor will run
         * @return Node
         */
        virtual Node macroNode(Node& node) = 0;

    protected:
        unsigned int m_debug;
        MacroProcessor* m_processor;  ///< This is a non-owned pointer.

        /**
         * @brief Find the nearest macro matching a giving name
         *
         * @details Proxy function for MacroProcessor::findNearestMacro
         *
         * @param name
         * @return const Node* nullptr if no macro was found
         */
        [[nodiscard]] const Node* findNearestMacro(const std::string& name) const;

        /**
         * @brief Apply a macro on a given node
         * @details Proxy function for MacroProcessor::applyMacro
         *
         * @param node
         * @param depth
         */
        void applyMacroProxy(Node& node, unsigned depth);

        /**
         * @brief Registers macros based on their type, expand conditional macros
         * @details Validate macros and register them by their name
         *  Proxy function for MacroProcessor::handleMacroNode
         *
         * @param node A node of type Macro
         * @param depth
         */
        void handleMacroNode(Node& node, unsigned depth) const;

        /**
         * @brief Check if a node can be evaluated to true
         * @details Proxy function for MacroProcessor::isTruthy
         *
         * @param node
         * @return true
         * @return false
         */
        [[nodiscard]] bool isTruthy(const Node& node) const;

        /**
         * @brief Evaluate only the macros
         * @details Proxy function for MacroProcessor::evaluate
         *
         * @param node
         * @param depth
         * @param is_not_body true if the method is run on a non-body code (eg a condition of an if-macro)
         * @return Node
         */
        Node evaluate(Node& node, unsigned depth, bool is_not_body) const;

        /**
         * @brief Throw a macro processing error
         * @details Proxy function for MacroProcessor::throwMacroProcessingError
         *
         * @param message the error
         * @param node the node in which there is an error
         */
        [[noreturn]] void throwMacroProcessingError(const std::string& message, const Node& node);
    };

}

#endif
