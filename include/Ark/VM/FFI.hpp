#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <iostream>
#include <sstream>

#include <Ark/VM/Value.hpp>
#include <Ark/Exceptions.hpp>

#define FFI_Function(name) Value name(const std::vector<Value>& n)

namespace Ark::internal::FFI
{
    extern const std::vector<std::string> builtins;

    // TODO annoter les fonctions qui n'utiliseront pas BUILTIN XY
    // TODO mettre des instructions spéciales pour les fonctions triviales (+-, cmp...)
    // TODO mettre nil, false et true dans les builtin

    FFI_Function(add);  // +, 2 arguments
    FFI_Function(sub);  // -, 2 arguments
    FFI_Function(mul);  // *, 2 arguments
    FFI_Function(div);  // /, 2 arguments

    FFI_Function(gt);   // >,  2 arguments
    FFI_Function(lt);   // <,  2 arguments
    FFI_Function(le);   // <=, 2 arguments
    FFI_Function(ge);   // >=, 2 arguments
    FFI_Function(neq);  // !=, 2 arguments
    FFI_Function(eq);   // =,  2 arguments

    FFI_Function(len);      // len,     1 argument
    FFI_Function(empty);    // empty?,  1 argument
    FFI_Function(firstof);  // firstof, 1 argument
    FFI_Function(tailof);   // tailof,  1 argument
    FFI_Function(append);   // append,  multiple arguments
    FFI_Function(concat);   // concat,  multiple arguments
    FFI_Function(list);     // list,    multiple arguments
    FFI_Function(isnil);    // nil?,    1 argument

    FFI_Function(print);    // print,  multiple arguments
    FFI_Function(assert_);  // assert, 2 arguments
    FFI_Function(input);    // input,  0 or 1 argument

    FFI_Function(toNumber);  // toNumber, 1 argument
    FFI_Function(toString);  // toString, 1 argument

    FFI_Function(at);      // @,      2 arguments
    FFI_Function(and_);    // and,    2 arguments
    FFI_Function(or_);     // or,     2 arguments
    FFI_Function(headof);  // headof, 1 argument

    FFI_Function(mod);  // mod, 2 arguments
}

#undef FFI_Function

#endif