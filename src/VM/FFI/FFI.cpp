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
        { "false",  falseSym },
        { "true",   trueSym },
        { "nil",    nil },

        { "append", Value(Array::append) },
        { "concat", Value(Array::concat) },
        { "list",   Value(Array::list) },

        { "print",  Value(IO::print) },
        { "input",  Value(IO::input) },
        { "writeFile", Value(IO::writeFile) },
        { "readFile", Value(IO::readFile) },
        { "fileExists?", Value(IO::fileExists) },

        { "time", Value(Time::timeSinceEpoch) },
        { "sleep", Value(Time::sleep) },

        { "system", Value(System::system_) },

        { "format", Value(String::format) }
    };

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
