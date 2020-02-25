#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>

#include <Ark/VM/FFI.hpp>

#define FFI_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const Value falseSym = Value(NFT::False);
    extern const Value trueSym  = Value(NFT::True);
    extern const Value nil      = Value(NFT::Nil);
    // not assignable value
    extern const Value undefined = Value(NFT::Undefined);

    namespace Mathematics
    {
        extern const Value pi_  = Value(M_PI);
        extern const Value e_   = Value(std::exp(1.0));
        extern const Value tau_ = Value(M_PI * 2.0);
        extern const Value inf_ = Value(HUGE_VAL);
        extern const Value nan_ = Value(std::numeric_limits<double>::signaling_NaN());
    }

    extern const std::vector<std::pair<std::string, Value>> builtins = {
        // builtin variables or constants
        { "false",  falseSym },
        { "true",   trueSym },
        { "nil",    nil },

        // List
        { "append", Value(List::append) },
        { "concat", Value(List::concat) },
        { "list",   Value(List::list) },
        { "reverseList", Value(List::reverseList) },
        { "findInList", Value(List::findInList) },
        { "removeAtList", Value(List::removeAtList) },
        { "sliceList", Value(List::sliceList) },
        { "sort", Value(List::sort_) },
        { "fill", Value(List::fill) },
        { "setListAt", Value(List::setListAt) },

        // IO
        { "print",  Value(IO::print) },
        { "puts", Value(IO::puts_) },
        { "input",  Value(IO::input) },
        { "writeFile", Value(IO::writeFile) },
        { "readFile", Value(IO::readFile) },
        { "fileExists?", Value(IO::fileExists) },
        { "listFiles", Value(IO::listFiles) },
        { "isDir?", Value(IO::isDirectory) },
        { "makeDir", Value(IO::makeDir) },
        { "removeFiles", Value(IO::removeFiles) },

        // Time
        { "time", Value(Time::timeSinceEpoch) },
        { "sleep", Value(Time::sleep) },

        // System
        { "system", Value(System::system_) },

        // String
        { "format", Value(String::format) },
        { "findSubStr", Value(String::findSubStr) },
        { "removeAtStr", Value(String::removeAtStr) },

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
        { "cos", Value(Mathematics::cos_) },
        { "sin", Value(Mathematics::sin_) },
        { "tan", Value(Mathematics::tan_) },
        { "arccos", Value(Mathematics::acos_) },
        { "arcsin", Value(Mathematics::asin_) },
        { "arctan", Value(Mathematics::atan_) }
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
        "not"
    };
}
