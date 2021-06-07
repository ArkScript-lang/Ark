/**
 * @file CValue.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The basic value type handled by the compiler
 * @version 0.2
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ARK_COMPILER_CVALUE_HPP
#define ARK_COMPILER_CVALUE_HPP

#include <variant>
#include <string>

#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
    /**
     * @brief Enumeration to keep track of the type of a C(ompiler)Value
     * 
     */
    enum class CValueType
    {
        Number,
        String,
        PageAddr  // for function definitions
    };

    /**
     * @brief A Compiler Value class helper to handle multiple types
     * 
     */
    struct CValue
    {
        std::variant<double, std::string, std::size_t> value;
        CValueType type;

        // Numbers
        explicit CValue(double value) noexcept;
        explicit CValue(long value) noexcept;
        // Strings
        explicit CValue(const std::string& value) noexcept;
        // automatic handling (Number/String/Function)
        explicit CValue(const Node& v) noexcept;
        // Functions
        explicit CValue(std::size_t value) noexcept;

        bool operator==(const CValue& A) noexcept;
    };
}

#endif
