#ifndef ark_ffi
#define ark_ffi

#include <vector>
#include <string>

namespace Ark
{
    namespace FFI
    {
        #define FFI_MAKE_EXTERNS_INC

        #include <Ark/MakeFFI.hpp>

        #undef FFI_MAKE_EXTERNS_INC
    }
}

#endif