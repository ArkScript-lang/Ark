#ifndef ark_compiler_value
#define ark_compiler_value

#include <variant>
#include <string>

#include <Ark/Compiler/Node.hpp>

namespace Ark::internal
{
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
        CValue(double value) noexcept;
        CValue(long value) noexcept;
        // Strings
        CValue(const std::string& value) noexcept;
        // automatic handling (Number/String/Function)
        CValue(const Node& v) noexcept;
        // Functions
        CValue(std::size_t value) noexcept;

        bool operator==(const CValue& A) noexcept;
    };
}

#endif