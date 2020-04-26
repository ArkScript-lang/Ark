#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <utility>
#include <sstream>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>

#define Builtins_Function(name) Value name(std::vector<Value>& n)

namespace Ark::internal::Builtins
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
        Builtins_Function(append);   // append, multiple arguments
        Builtins_Function(concat);   // concat, multiple arguments
        Builtins_Function(list);     // list,   multiple arguments
        Builtins_Function(reverseList);  // reverseList, single arg
        Builtins_Function(findInList);   // findInList, 2 arguments
        Builtins_Function(removeAtList);  // removeAtList, 2 arguments
        Builtins_Function(sliceList);    // sliceList, 4 arguments
        Builtins_Function(sort_);  // sort, 1 argument
        Builtins_Function(fill);  // fill, 2 arguments
        Builtins_Function(setListAt);  // setListAt, 3 arguments
    }

    namespace IO
    {
        Builtins_Function(print);    // print, multiple arguments
        Builtins_Function(puts_);    // puts, multiple arguments
        Builtins_Function(input);    // input, 0 or 1 argument
        Builtins_Function(writeFile);   // writeFile, 2 or 3 arguments
        Builtins_Function(readFile);    // readFile, 1 argument
        Builtins_Function(fileExists);  // fileExists?, 1 argument
        Builtins_Function(listFiles);  // listFiles, 1 argument
        Builtins_Function(isDirectory);  // isDir?, 1 argument
        Builtins_Function(makeDir);  // makeDir, 1 argument
        Builtins_Function(removeFiles);  // removeFiles, multiple arguments
    }

    namespace Time
    {
        Builtins_Function(timeSinceEpoch);  // time, 0 argument
        Builtins_Function(sleep);  // sleep, 1 argument
    }

    namespace System
    {
        Builtins_Function(system_);  // system, 1 argument
    }

    namespace String
    {
        Builtins_Function(format);  // format, multiple arguments
        Builtins_Function(findSubStr);  // findSubStr, 2 arguments
        Builtins_Function(removeAtStr);  // removeAtStr, 2 arguments
    }

    namespace Mathematics
    {
        Builtins_Function(exponential);  // exp, 1 argument
        Builtins_Function(logarithm);  // ln, 1 argument
        Builtins_Function(ceil_);  // ceil, 1 argument
        Builtins_Function(floor_);  // floor, 1 argument
        Builtins_Function(round_);  // round, 1 argument
        Builtins_Function(isnan_);  // NaN?, 1 argument
        Builtins_Function(isinf_);  // Inf?, 1 argument

        extern const Value pi_;
        extern const Value e_;
        extern const Value tau_;
        extern const Value inf_;
        extern const Value nan_;

        Builtins_Function(cos_);  // cos, 1 argument
        Builtins_Function(sin_);  // sin, 1 argument
        Builtins_Function(tan_);  // tan, 1 argument
        Builtins_Function(acos_);  // arccos, 1 argument
        Builtins_Function(asin_);  // arcsin, 1 argument
        Builtins_Function(atan_);  // arctan, 1 argument
    }
}

#undef Builtins_Function

#endif
