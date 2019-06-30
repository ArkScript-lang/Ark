#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <utility>
#include <iostream>
#include <sstream>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

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
    FFI_Function(append);   // append, multiple arguments
    FFI_Function(concat);   // concat, multiple arguments
    FFI_Function(list);     // list,   multiple arguments
    FFI_Function(print);    // print,  multiple arguments
    FFI_Function(input);    // input,  0 or 1 argument
}

#undef FFI_Function

#endif