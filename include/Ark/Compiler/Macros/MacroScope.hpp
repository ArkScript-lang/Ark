/**
 * @file MacroScope.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Defines tools to handle macro definitions
 * @version 0.1
 * @date 2023-02-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef COMPILER_MACROS_SCOPE_HPP
#define COMPILER_MACROS_SCOPE_HPP

#include <Ark/Compiler/AST/Node.hpp>

#include <unordered_map>
#include <string>

namespace Ark::internal
{
    class MacroScope
    {
    public:
        MacroScope();

        /**
         * @brief Construct a new MacroScope object given a depth in the scope hierarchy
         * 
         * @param depth 
         */
        explicit MacroScope(unsigned int depth);

        /**
         * @brief Check if the current scope holds a value for a given symbol, and returns it as a pointer
         * 
         * @param name 
         * @return Node* pointer to the value if found, nullptr otherwise
         */
        const Node* has(const std::string& name) const;

        /**
         * @brief Add a new entry in the scope
         * 
         * @param name 
         * @param node 
         */
        void add(const std::string& name, const Node& node);

        /**
         * @brief Remove a macro in the scope, if it exists
         * 
         * @param name 
         * @return true if the value was found and removed
         * @return false otherwise
         */
        bool remove(const std::string& name);

        /**
         * @brief Return true if the current scope is empty
         * 
         * @return true 
         * @return false 
         */
        inline bool empty() const
        {
            return m_macros.empty();
        }

        inline unsigned int depth() const
        {
            return m_depth;
        }

    private:
        std::unordered_map<std::string, Node> m_macros;
        unsigned int m_depth;
    };
}

#endif
