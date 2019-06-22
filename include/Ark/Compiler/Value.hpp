#ifndef ark_compiler_value
#define ark_compiler_value

#include <variant>
#include <string>

#include <Ark/Parser/Node.hpp>

namespace Ark
{
    namespace internal
    {
        enum class CValueType
        {
            Number,
            String,
            PageAddr  // for function definitions
        };

        struct CValue
        {
            std::variant<double, std::string, std::size_t> value;
            CValueType type;

            CValue(double value);
            CValue(long value);
            CValue(const std::string& value);
            CValue(const Node& v);
            CValue(std::size_t value);

            bool operator==(const CValue& A);
        };
    }
}

#endif