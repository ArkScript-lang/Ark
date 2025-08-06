/**
 * @file ExecutionContext.hpp
 * @author Lex Plateau (lexplt.dev@gmail.com)
 * @brief Keeping track of the internal data needed by the VM
 * @date 2021-11-15
 *
 * @copyright Copyright (c) 2021-2025
 *
 */

#ifndef ARK_VM_EXECUTIONCONTEXT_HPP
#define ARK_VM_EXECUTIONCONTEXT_HPP

#include <array>
#include <limits>
#include <memory>
#include <optional>
#include <atomic>

#include <Ark/Constants.hpp>
#include <Ark/VM/Value.hpp>
#include <Ark/VM/ScopeView.hpp>
#include <Ark/VM/Value/ClosureScope.hpp>

#ifdef max
#    undef max
#endif

namespace Ark::internal
{
    struct ExecutionContext
    {
        static inline unsigned Count = 0;

        std::size_t ip {};  ///< Instruction pointer
        std::size_t pp {};  ///< Page pointer
        uint16_t sp {};     ///< Stack pointer
        uint16_t fc {};     ///< Frame count
        uint16_t last_symbol;
        const bool primary;  ///< Tells if the current ExecutionContext is the primary one or not
        std::atomic_bool active;

        std::optional<ClosureScope> saved_scope {};                            ///< Scope created by CAPTURE <x> instructions, used by the MAKE_CLOSURE instruction
        std::vector<std::shared_ptr<ClosureScope>> stacked_closure_scopes {};  ///< Stack the closure scopes to keep the closure alive as long as we are calling them

        std::vector<ScopeView> locals {};
        std::array<ScopeView::pair_t, ScopeStackSize> scopes_storage {};  ///< All the ScopeView use this array to store id->value

        std::array<Value, VMStackSize> stack {};

        ExecutionContext() noexcept :
            last_symbol(std::numeric_limits<uint16_t>::max()),
            primary(Count == 0)
        {
            active.store(true);
            Count++;
        }

        [[nodiscard]] bool isFree() const
        {
            return !active.load();
        }

        void setActive(const bool toggle)
        {
            active.store(toggle);
        }
    };
}

#endif
