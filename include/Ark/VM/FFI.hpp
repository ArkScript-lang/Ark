#ifndef ark_vm_ffi
#define ark_vm_ffi

#include <vector>
#include <iostream>
#include <sstream>

#include <Ark/VM/Value.hpp>

namespace Ark
{
    namespace internal
    {
        namespace FFI
        {
            #define FFI_VM
            #define FFI_MAKE_HEADER

            #include <Ark/MakeFFI.hpp>

            #undef FFI_VM
            #undef FFI_MAKE_HEADER
        }
    }
}

#endif