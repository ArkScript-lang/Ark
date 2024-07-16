/**
 * @file Executor.hpp
 * @author Ray John Alovera (rakista112@gmail.com), Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The base class for all MacroExecutors
 * @version 2.0
 * @date 2024-03-03
 *
 * @copyright Copyright (c) 2021-2024
 *
 */

#ifndef ARK_COMPILER_EXECUTOR_HPP
#define ARK_COMPILER_EXECUTOR_HPP

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

    protected:
        unsigned int m_debug;
        MacroProcessor* m_processor;  ///< This is a non-owned pointer.

        void setWithFileAttributes(const Node origin, Node& output, const Node& macro);

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
         * @brief Registers macros based on their type
         * @details Validate macros and register them by their name
         *  Proxy function for MacroProcessor::registerMacro
         *
         * @param node A node of type Macro
         */
        void registerMacro(Node& node) const;

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
         * @brief Applies the spread operator
         * @details Proxy function for MacroProcessor::unify
         */
        void unify(const std::unordered_map<std::string, Node>&, Node&, Node*) const;

        /**
         * @brief Throw a macro processing error
         * @details Proxy function for MacroProcessor::throwMacroProcessingError
         *
         * @param message the error
         * @param node the node in which there is an error
         */
        [[noreturn]] void throwMacroProcessingError(const std::string& message, const Node& node) const;

        /**
         * @brief Execute a node, trying to emplace macros calls
         * @details Proxy function for MacroProcessor::applyMacro
         *
         * @param node
         * @param depth
         * @return true
         * @return false
         */
        bool applyMacroProxy(Node& node, unsigned depth) const;

        /**
         * @brief Check if a given symbol is a predefined macro
         * @details Proxy function for MacroProcessor::isPredefined
         *
         * @param symbol
         * @return true
         * @return false
         */
        [[nodiscard]] bool isPredefined(const std::string& symbol) const;
    };

}

#endif
