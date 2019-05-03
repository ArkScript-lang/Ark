#ifndef ark_lib
#define ark_lib

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#include <Ark/Lang/Node.hpp>
#include <Ark/Lang/Environment.hpp>

namespace Ark
{
    namespace Lang
    {
        #define FFI_INTERPRETER
        #define FFI_MAKE_HEADER

        #include <Ark/MakeFFI.hpp>

        #undef FFI_INTERPRETER
        #undef FFI_MAKE_HEADER
    }
}

#undef FUNCTION

#endif  // ark_lib
