#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>

#include <Ark/VM/Value.hpp>

#define FUNCTION(name) Value name(const std::vector<Value>& n);

namespace Ark
{
    namespace VM
    {
        namespace FFI
        {
            FUNCTION(add)  // +
            FUNCTION(sub)  // -
            FUNCTION(mul)  // *
            FUNCTION(div)  // /

            FUNCTION(gt)  // >
            FUNCTION(lt)  // <
            FUNCTION(le)  // <=
            FUNCTION(ge)  // >=
            FUNCTION(neq)  // !=
            FUNCTION(eq)  // =

            FUNCTION(len)  // len 1
            FUNCTION(empty)  // empty? 1
            FUNCTION(firstof)  // firstof 1
            FUNCTION(tailof)  // tailof +
            FUNCTION(append)  // append +
            FUNCTION(concat)  // concat +
            FUNCTION(list)  // list +
            FUNCTION(isnil)  // nil? 1

            FUNCTION(print)  // print +
            FUNCTION(assert_)  // assert 2
        }
    }
}

#undef FUNCTION

#endif