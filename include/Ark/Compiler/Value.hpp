#ifndef ark_compiler_value
#define ark_compiler_value

#include <variant>
#include <string>

#include <huge_number.hpp>

namespace Ark
{
    namespace Compiler
    {
        enum class ValueType
        {
            Number,
            String
        };

        struct Value
        {
            std::variant<dozerg::HugeNumber, std::string> value;
            ValueType type;

            template <typename T>
            Value(const T& value);

            bool operator==(const Value& A);
        };
    }
}

#endif