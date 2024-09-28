/**
 * @file ValTableElem.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The basic value type handled by the compiler
 * @version 1.0
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_COMPILER_VALTABLEELEM_HPP
#define ARK_COMPILER_VALTABLEELEM_HPP

#include <variant>
#include <string>

#include <Ark/Compiler/AST/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Enumeration to keep track of the type of a Compiler Value
     *
     */
    enum class ValTableElemType
    {
        Number,
        String,
        PageAddr  // for function definitions
    };

    /**
     * @brief A Compiler Value class helper to handle multiple types
     *
     */
    struct ValTableElem
    {
        std::variant<double, std::string, std::size_t> value;
        ValTableElemType type;

        // automatic handling (Number/String/Function)
        explicit ValTableElem(const Node& node) noexcept;
        // Functions
        explicit ValTableElem(std::size_t page) noexcept;

        bool operator==(const ValTableElem& A) const noexcept;
    };
}

#endif
