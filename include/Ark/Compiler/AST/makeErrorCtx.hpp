/**
 * @file makeErrorCtx.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Create string error context for AST errors
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2021
 *
 */

#ifndef COMPILER_AST_MAKEERRORCTX_HPP
#define COMPILER_AST_MAKEERRORCTX_HPP

#include <sstream>
#include <string>

#include <Ark/Compiler/AST/Node.hpp>
#include <termcolor/termcolor.hpp>

namespace Ark::internal
{
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
}

#endif
