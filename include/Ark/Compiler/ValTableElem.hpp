/**
 * @file ValTableElem.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief The basic value type handled by the compiler
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
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
     * @brief Enumeration to keep track of the type of Compiler Value
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
        // Numbers
        explicit ValTableElem(double n) noexcept;
        // Strings
        explicit ValTableElem(const std::string& str) noexcept;
        // Functions
        explicit ValTableElem(std::size_t page) noexcept;

        bool operator==(const ValTableElem& A) const noexcept;
    };
}

#endif
