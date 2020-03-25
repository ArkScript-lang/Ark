#ifndef ark_vm_types
#define ark_vm_types

#include <cinttypes>

namespace Ark::internal
{
    class Nil   {};
    class False {};
    class True  {};

    using PageAddr_t = uint16_t;
}

#endif