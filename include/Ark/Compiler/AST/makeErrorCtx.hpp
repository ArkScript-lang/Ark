/**
 * @file makeErrorCtx.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Create string error context for AST errors
 * @version 0.2
 * @date 2022-02-19
 *
 * @copyright Copyright (c) 2020-2022
 *
 */

#ifndef COMPILER_AST_MAKEERRORCTX_HPP
#define COMPILER_AST_MAKEERRORCTX_HPP

#include <sstream>
#include <string>

#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    struct LineColorContextCounts
    {
        int open_parentheses = 0;
        int open_square_braces = 0;
        int open_curly_braces = 0;
    };

    /**
     * @brief Construct an error message based on a given node
     * @details It opens the related file at the line and column of the node,
     *          and display context, plus underline the problem with a serie of ^.
     *
     * @param message
     * @param node
     * @return std::string the complete generated error message
     */
    std::string makeNodeBasedErrorCtx(const std::string& message, const Node& node);

    /**
     * @brief Construct an error message based on a given match in the code
     * @details Mostly used by the Lexer and Parser since they don't have Nodes to work on
     *
     * @param match the identified token, causing a problem
     * @param line line of the token
     * @param col starting column of the token
     * @param code the whole code of the file
     * @return std::string the complete generated error message
     */
    std::string makeTokenBasedErrorCtx(const std::string& match, std::size_t line, std::size_t col, const std::string& code);

    /**
     * @brief Add colors to highlight matching parentheses/curly braces/square braces on a line
     *
     * @param line the line of code to colorize
     * @param line_color_context_counts a LineColorContextCounts to manipulate the running counts of open pairings
     * @return std::string a colorized line of code
     */
    std::string colorizeLine(const std::string& line, LineColorContextCounts& line_color_context_counts);

    /**
     * @brief Check if the character passed in can be paired (parentheses, curly braces, or square braces)
     *
     * @param c
     * @return bool
     */
    inline bool isPairableChar(const char c)
    {
        return c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}';
    }
}

#endif
