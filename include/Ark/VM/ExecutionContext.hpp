/**
 * @file ExecutionContext.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief Keeping track of the internal data needed by the VM
 * @version 0.1
 * @date 2021-11-15
 *
 * @copyright Copyright (c) 2021-2024
 *
 */

#ifndef ARK_VM_EXECUTIONCONTEXT_HPP
#define ARK_VM_EXECUTIONCONTEXT_HPP

#include <array>
#include <limits>
#include <memory>
#include <optional>

#include <Ark/Constants.hpp>
#include <Ark/VM/Value.hpp>
#include <Ark/VM/Scope.hpp>

#ifdef max
#    undef max
#endif

namespace Ark::internal
{
    struct ExecutionContext
    {
        static inline unsigned Count = 0;

        const bool primary;  ///< Tells if the current ExecutionContext is the primary one or not
        std::size_t ip {};   ///< Instruction pointer
        std::size_t pp {};   ///< Page pointer
        uint16_t sp {};      ///< Stack pointer
        uint16_t fc {};      ///< Frame count
        uint16_t last_symbol;

        std::array<Value, VMStackSize> stack {};
        std::vector<std::shared_ptr<Scope>> stacked_closure_scopes {};  ///< Stack the closure scopes to keep the closure alive as long as we are calling them
        std::optional<Scope> saved_scope {};                            ///< Scope created by CAPTURE <x> instructions, used by the MAKE_CLOSURE instruction
        std::vector<Scope> locals {};

        ExecutionContext() noexcept :
            primary(Count == 0),
            last_symbol(std::numeric_limits<uint16_t>::max())
        {
            Count++;
        }
    };
}

#endif
