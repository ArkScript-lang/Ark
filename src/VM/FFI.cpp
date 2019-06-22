#include <Ark/VM/FFI.hpp>

#include <iostream>
#include <Ark/Log.hpp>
#undef abs
#include <cmath>

namespace Ark::internal
{
    namespace FFI
    {
        #define FFI_VM
        #define FFI_MAKE_SOURCE

        #include <Ark/MakeFFI.hpp>

        #undef FFI_VM
        #undef FFI_MAKE_SOURCE
    }
}
