#ifndef ark_compiler_value
#define ark_compiler_value

#include <variant>
#include <string>

#include <Ark/Parser/Node.hpp>

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
        CValue(double value);
        CValue(long value);
        // Strings
        CValue(const std::string& value);
        // automatic handling (Number/String/Function)
        CValue(const Node& v);
        // Functions
        CValue(std::size_t value);

        bool operator==(const CValue& A);
    };
}

#endif