#include <Ark/Lang/Lib.hpp>

#include <iostream>
#include <Ark/Log.hpp>
#undef abs
#include <cmath>

namespace Ark
{
    namespace Lang
    {
        #define FFI_INTERPRETER
        #define FFI_MAKE_SOURCE

        #include <Ark/MakeFFI.hpp>

        #undef FFI_INTERPRETER
        #undef FFI_MAKE_SOURCE
    }
}
