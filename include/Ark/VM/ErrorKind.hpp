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
        Scope,
        Type,
        Index,
        Arity,
        DivisionByZero,
    };

    constexpr std::array<std::string_view, 7> errorKinds = {
        "VMError",
        "ModuleError",
        "ScopeError",
        "TypeError",
        "IndexError",
        "ArityError",
        "DivisionByZero"
    };
}

#endif
