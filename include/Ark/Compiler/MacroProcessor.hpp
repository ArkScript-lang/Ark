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
        void feed(Node& ast);

        /**
         * @brief Return the modified AST
         * 
         * @return Node& 
         */
        Node& ast() noexcept;

    private:
        unsigned m_debug;  ///< The debug level
        uint16_t m_options;
        Node* m_ast;  ///< The modified AST
        std::vector<std::unordered_map<std::string, Node>> m_macros;  ///< Handling macros in a scope fashion

        /**
         * @brief Registers macros based on their type
         * @details Validate macros and register them by their name
         * 
         * @param node A node of type Macro
         */
        void registerMacro(Node* node);

        /**
         * @brief Register macros in scopes and apply them as needed
         * 
         * @param node node on which to operate
         */
        void process(Node* node);

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
