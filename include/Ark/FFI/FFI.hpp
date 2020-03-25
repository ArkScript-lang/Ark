#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <utility>
#include <sstream>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>

#define FFI_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const Value falseSym;
    extern const Value trueSym;
    extern const Value nil;

    extern const std::vector<std::pair<std::string, Value>> builtins;
    extern const std::vector<std::string> operators;

    // ------------------------------
    // builtins functions: we must use the instruction BUILTIN index
    // ------------------------------
    namespace List
    {
        FFI_Function(append);   // append, multiple arguments
        FFI_Function(concat);   // concat, multiple arguments
        FFI_Function(list);     // list,   multiple arguments
        FFI_Function(reverseList);  // reverseList, single arg
        FFI_Function(findInList);   // findInList, 2 arguments
        FFI_Function(removeAtList);  // removeAtList, 2 arguments
        FFI_Function(sliceList);    // sliceList, 4 arguments
        FFI_Function(sort_);  // sort, 1 argument
        FFI_Function(fill);  // fill, 2 arguments
        FFI_Function(setListAt);  // setListAt, 3 arguments
    }

    namespace IO
    {
        FFI_Function(print);    // print, multiple arguments
        FFI_Function(puts_);    // puts, multiple arguments
        FFI_Function(input);    // input, 0 or 1 argument
        FFI_Function(writeFile);   // writeFile, 2 or 3 arguments
        FFI_Function(readFile);    // readFile, 1 argument
        FFI_Function(fileExists);  // fileExists?, 1 argument
        FFI_Function(listFiles);  // listFiles, 1 argument
        FFI_Function(isDirectory);  // isDir?, 1 argument
        FFI_Function(makeDir);  // makeDir, 1 argument
        FFI_Function(removeFiles);  // removeFiles, multiple arguments
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
        FFI_Function(removeAtStr);  // removeAtStr, 2 arguments
    }

    namespace Mathematics
    {
        FFI_Function(exponential);  // exp, 1 argument
        FFI_Function(logarithm);  // ln, 1 argument
        FFI_Function(ceil_);  // ceil, 1 argument
        FFI_Function(floor_);  // floor, 1 argument
        FFI_Function(round_);  // round, 1 argument
        FFI_Function(isnan_);  // NaN?, 1 argument
        FFI_Function(isinf_);  // Inf?, 1 argument

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
