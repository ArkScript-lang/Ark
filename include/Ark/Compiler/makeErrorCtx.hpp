#ifndef ARK_COMPILER_MAKEERRORCTX_HPP
#define ARK_COMPILER_MAKEERRORCTX_HPP

#include <sstream>
#include <string>

#include <Ark/Compiler/Node.hpp>

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
    std::string makeNodeBasedErrorCtx(const std::string& message, const Ark::internal::Node& node);

    std::string makeTokenBasedErrorCtx(const std::string& match, std::size_t line, std::size_t col, const std::string& code);
}

#endif
