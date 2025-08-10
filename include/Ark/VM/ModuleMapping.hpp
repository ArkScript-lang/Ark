#ifndef ARK_VM_MODULEMAPPING_HPP
#define ARK_VM_MODULEMAPPING_HPP

#include <Ark/VM/VM.hpp>

namespace Ark
{
    struct mapping
    {
        const char* name;
        Value (*value)(std::vector<Value>&, VM*);
    };
}

#endif  // ARK_VM_MODULEMAPPING_HPP
