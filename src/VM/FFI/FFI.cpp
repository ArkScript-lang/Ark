#include <Ark/VM/FFI.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const Value falseSym = Value(NFT::False);
    extern const Value trueSym  = Value(NFT::True);
    extern const Value nil      = Value(NFT::Nil);
    // not assignable value
    extern const Value undefined = Value(NFT::Undefined);

    extern const std::vector<std::pair<std::string, Value>> builtins = {
        // builtin variables or constants
        { "false",  falseSym },
        { "true",   trueSym },
        { "nil",    nil },

        // Array
        { "append", Value(Array::append) },
        { "concat", Value(Array::concat) },
        { "list",   Value(Array::list) },

        // IO
        { "print",  Value(IO::print) },
        { "input",  Value(IO::input) },
        { "writeFile", Value(IO::writeFile) },
        { "readFile", Value(IO::readFile) },
        { "fileExists?", Value(IO::fileExists) },

        // Time
        { "time", Value(Time::timeSinceEpoch) },
        { "sleep", Value(Time::sleep) },

        // System
        { "system", Value(System::system_) },

        // String
        { "format", Value(String::format) },

        // Mathematics
        { "exp", Value(Mathematics::exponential) },
        { "ln", Value(Mathematics::logarithm) },
        { "ceil", Value(Mathematics::ceil_) },
        { "floor", Value(Mathematics::floor_) },
        { "round", Value(Mathematics::round_) },
        { "isNaN", Value(Mathematics::isnan_) },
        { "isInf", Value(Mathematics::isinf_) },
        { "Pi", Mathematics::pi_ },
        { "E", Mathematics::e_ },
        { "Tau", Mathematics::tau_ },
        { "Inf", Mathematics::inf_ },
        { "NaN", Mathematics::nan_ },
        { "cos", Mathematics::cos_ },
        { "sin", Mathematics::sin_ },
        { "tan", Mathematics::tan_ },
        { "arccos", Mathematics::acos_ },
        { "arcsin", Mathematics::asin_ },
        { "arctan", Mathematics::atan_ }
    };

    // This list is related to include/Ark/Compiler/Instructions.hpp
    // from FIRST_OPERATOR, to LAST_OPERATOR
    // The order is very important
    extern const std::vector<std::string> operators = {
        "+", "-", "*", "/",
        ">", "<", "<=", ">=", "!=", "=",
        "len", "empty?", "firstOf", "tailOf", "headOf",
        "nil?", "assert",
        "toNumber", "toString",
        "@", "and", "or", "mod",
        "type", "hasField",
    };
}
