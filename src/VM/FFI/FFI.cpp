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
        { "append", Value(append) },
        { "concat", Value(concat) },
        { "list",   Value(list) },
        { "print",  Value(print) },
        { "input",  Value(input) },
        { "writeFile", Value(writeFile) },
        { "readFile", Value(readFile) },
        { "fileExists?", Value(fileExists) },
        { "time", Value(timeSinceEpoch) },
        { "sleep", Value(sleep) },
        { "system", Value(system_) },
        { "format", Value(format) }
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
