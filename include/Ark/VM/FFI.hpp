#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <utility>
#include <sstream>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const Value falseSym;
    extern const Value trueSym;
    extern const Value nil;
    extern const Value undefined;  // internal value only

    extern const std::vector<std::pair<std::string, Value>> builtins;
    extern const std::vector<std::string> operators;

    // ------------------------------
    // builtins functions: we must use the instruction BUILTIN index
    // ------------------------------
    namespace Array
    {
        FFI_Function(append);   // append, multiple arguments
        FFI_Function(concat);   // concat, multiple arguments
        FFI_Function(list);     // list,   multiple arguments
    }

    namespace IO
    {
        FFI_Function(print);    // print,  multiple arguments
        FFI_Function(input);    // input,  0 or 1 argument
        FFI_Function(writeFile);   // writeFile, 2 or 3 arguments
        FFI_Function(readFile);    // readFile, 1 argument
        FFI_Function(fileExists);  // fileExists?, 1 argument
    }

    namespace Time
    {
        FFI_Function(timeSinceEpoch);  // time, 0 argument
        FFI_Function(sleep);  // sleep, 1 argument
    }

    namespace System
    {
        FFI_Function(system_);  // system, 1 argument
    }

    namespace String
    {
        FFI_Function(format);  // format, multiple arguments
        FFI_Function(findSubStr);  // findSubStr, 2 arguments
    }

    namespace Mathematics
    {
        FFI_Function(exponential);  // exp, 1 argument
        FFI_Function(logarithm);  // ln, 1 argument
        FFI_Function(ceil_);  // ceil, 1 argument
        FFI_Function(floor_);  // floor, 1 argument
        FFI_Function(round_);  // round, 1 argument
        FFI_Function(isnan_);  // isNaN, 1 argument
        FFI_Function(isinf_);  // isInf, 1 argument

        extern const Value pi_;
        extern const Value e_;
        extern const Value tau_;
        extern const Value inf_;
        extern const Value nan_;

        FFI_Function(cos_);  // cos, 1 argument
        FFI_Function(sin_);  // sin, 1 argument
        FFI_Function(tan_);  // tan, 1 argument
        FFI_Function(acos_);  // arccos, 1 argument
        FFI_Function(asin_);  // arcsin, 1 argument
        FFI_Function(atan_);  // arctan, 1 argument
    }
}

#undef FFI_Function

#endif