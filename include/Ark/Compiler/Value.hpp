#ifndef ark_compiler_value
#define ark_compiler_value

#include <variant>
#include <string>

#include <Ark/Parser/Node.hpp>

namespace Ark
{
    namespace Compiler
    {
        enum class ValueType
        {
            Number,
            String,
            PageAddr  // for function definitions
        };

        struct Value
        {
            std::variant<double, std::string, std::size_t> value;
            ValueType type;

            Value(double value);
            Value(long value);
            Value(const std::string& value);
            Value(const Ark::Parser::Node& v);
            Value(std::size_t value);

            bool operator==(const Value& A);
        };
    }
}

#endif