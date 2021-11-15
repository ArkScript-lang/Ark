#ifndef ARK_VM_VMRUNTIME_HPP
#define ARK_VM_VMRUNTIME_HPP

#include <array>
#include <limits>
#include <optional>
#include <cinttypes>

#include <Ark/Constants.hpp>
#include <Ark/VM/Value.hpp>
#include <Ark/VM/Scope.hpp>

namespace Ark::internal
{
    struct ExecutionHandler
    {
        int ip = 0;          ///< Instruction pointer
        std::size_t pp = 0;  ///< Page pointer
        uint16_t sp = 0;     ///< Stack pointer
        uint16_t last_symbol = std::numeric_limits<uint16_t>::max();

        std::array<Value, VMStackSize> stack;
        std::vector<uint8_t> scope_count_to_delete;
        std::optional<Scope_t> saved_scope;
        std::vector<Scope_t> locals;
    };
}

#endif
