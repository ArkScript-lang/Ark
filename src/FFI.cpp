#include <Ark/FFI.hpp>

namespace Ark
{
    namespace FFI
    {
        #define FFI_MAKE_EXTERNS_SRC

        #include <Ark/MakeFFI.hpp>

        #undef FFI_MAKE_EXTERNS_SRC
    }
}