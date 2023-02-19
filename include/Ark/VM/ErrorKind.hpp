#ifndef ARK_VM_ERRORKIND_HPP
#define ARK_VM_ERRORKIND_HPP

#include <array>
#include <string_view>

namespace Ark::internal
{
    enum class ErrorKind
    {
        VM,
        Module,
        Mutability,
        Scope,
        Type,
        Index,
        Arity,
    };

    constexpr std::array<std::string_view, 7> errorKinds = {
        "VMError",
        "ModuleError",
        "MutabilityError",
        "ScopeError",
        "TypeError",
        "IndexError",
        "ArityError"
    };
}

#endif
