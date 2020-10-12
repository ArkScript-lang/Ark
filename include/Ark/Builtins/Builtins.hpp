#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <utility>
#include <sstream>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>
#include <Ark/Utils.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal::Builtins
{
    extern const Value falseSym;
    extern const Value trueSym;
    extern const Value nil;

    extern const std::vector<std::pair<std::string, Value>> builtins;
    extern const std::vector<std::string> operators;

    // ------------------------------
    // builtins functions: we must use the instruction `BUILTIN index`
    // ------------------------------
    namespace List
    {
        Value reverseList(std::vector<Value>& n, Ark::VM* vm);  // reverseList, single arg
        Value findInList(std::vector<Value>& n, Ark::VM* vm);   // findInList, 2 arguments
        Value removeAtList(std::vector<Value>& n, Ark::VM* vm);  // removeAtList, 2 arguments
        Value sliceList(std::vector<Value>& n, Ark::VM* vm);    // sliceList, 4 arguments
        Value sort_(std::vector<Value>& n, Ark::VM* vm);  // sort, 1 argument
        Value fill(std::vector<Value>& n, Ark::VM* vm);  // fill, 2 arguments
        Value setListAt(std::vector<Value>& n, Ark::VM* vm);  // setListAt, 3 arguments
    }

    namespace IO
    {
        Value print(std::vector<Value>& n, Ark::VM* vm);    // print, multiple arguments
        Value puts_(std::vector<Value>& n, Ark::VM* vm);    // puts, multiple arguments
        Value input(std::vector<Value>& n, Ark::VM* vm);    // input, 0 or 1 argument
        Value writeFile(std::vector<Value>& n, Ark::VM* vm);   // writeFile, 2 or 3 arguments
        Value readFile(std::vector<Value>& n, Ark::VM* vm);    // readFile, 1 argument
        Value fileExists(std::vector<Value>& n, Ark::VM* vm);  // fileExists?, 1 argument
        Value listFiles(std::vector<Value>& n, Ark::VM* vm);  // listFiles, 1 argument
        Value isDirectory(std::vector<Value>& n, Ark::VM* vm);  // isDir?, 1 argument
        Value makeDir(std::vector<Value>& n, Ark::VM* vm);  // makeDir, 1 argument
        Value removeFiles(std::vector<Value>& n, Ark::VM* vm);  // removeFiles, multiple arguments
    }

    namespace Time
    {
        Value timeSinceEpoch(std::vector<Value>& n, Ark::VM* vm);  // time, 0 argument
    }

    namespace System
    {
        Value system_(std::vector<Value>& n, Ark::VM* vm);  // system, 1 argument
        Value sleep(std::vector<Value>& n, Ark::VM* vm);  // sleep, 1 argument
    }

    namespace String
    {
        Value format(std::vector<Value>& n, Ark::VM* vm);  // format, multiple arguments
        Value findSubStr(std::vector<Value>& n, Ark::VM* vm);  // findSubStr, 2 arguments
        Value removeAtStr(std::vector<Value>& n, Ark::VM* vm);  // removeAtStr, 2 arguments
    }

    namespace Mathematics
    {
        Value exponential(std::vector<Value>& n, Ark::VM* vm);  // exp, 1 argument
        Value logarithm(std::vector<Value>& n, Ark::VM* vm);  // ln, 1 argument
        Value ceil_(std::vector<Value>& n, Ark::VM* vm);  // ceil, 1 argument
        Value floor_(std::vector<Value>& n, Ark::VM* vm);  // floor, 1 argument
        Value round_(std::vector<Value>& n, Ark::VM* vm);  // round, 1 argument
        Value isnan_(std::vector<Value>& n, Ark::VM* vm);  // NaN?, 1 argument
        Value isinf_(std::vector<Value>& n, Ark::VM* vm);  // Inf?, 1 argument

        extern const Value pi_;
        extern const Value e_;
        extern const Value tau_;
        extern const Value inf_;
        extern const Value nan_;

        Value cos_(std::vector<Value>& n, Ark::VM* vm);  // cos, 1 argument
        Value sin_(std::vector<Value>& n, Ark::VM* vm);  // sin, 1 argument
        Value tan_(std::vector<Value>& n, Ark::VM* vm);  // tan, 1 argument
        Value acos_(std::vector<Value>& n, Ark::VM* vm);  // arccos, 1 argument
        Value asin_(std::vector<Value>& n, Ark::VM* vm);  // arcsin, 1 argument
        Value atan_(std::vector<Value>& n, Ark::VM* vm);  // arctan, 1 argument
    }
}

#undef Builtins_Function

#endif
