/**
 * @file ValTableElem.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The basic value type handled by the compiler
 * @version 0.2
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
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
     * @brief Enumeration to keep track of the type of a C(ompiler)Value
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

        // Numbers
        explicit ValTableElem(double value) noexcept;
        explicit ValTableElem(long value) noexcept;
        // Strings
        explicit ValTableElem(const std::string& value) noexcept;
        // automatic handling (Number/String/Function)
        explicit ValTableElem(const Node& v) noexcept;
        // Functions
        explicit ValTableElem(std::size_t value) noexcept;

        bool operator==(const ValTableElem& A) noexcept;
    };
}

#endif
