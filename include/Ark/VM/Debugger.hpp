/**
 * @file Debugger.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief Debugger used by the VM when an error or a breakpoint is reached
 * @date 2026-01-12
 *
 * @copyright Copyright (c) 2026-01-12
 *
 */

#ifndef ARK_VM_DEBUGGER_HPP
#define ARK_VM_DEBUGGER_HPP

#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <filesystem>

#include <Ark/VM/Value/Value.hpp>
#include <Ark/VM/ExecutionContext.hpp>

namespace Ark
{
    class VM;
}

namespace Ark::internal
{
    struct SavedState
    {
        std::size_t ip;
        std::size_t pp;
        uint16_t sp;
        uint16_t fc;
        std::vector<ScopeView> locals;
        std::vector<std::shared_ptr<ClosureScope>> closure_scopes;
    };

    class Debugger
    {
    public:
        /**
         * @brief Create a new Debugger object
         *
         * @param context context from the VM before displaying a backtrace
         * @param libenv
         * @param symbols symbols table of the VM
         * @param constants constants table of the VM
         */
        explicit Debugger(const ExecutionContext& context, const std::vector<std::filesystem::path>& libenv, const std::vector<std::string>& symbols, const std::vector<Value>& constants);

        /**
         * @brief Save the current VM state, to get back to it once the debugger is done running
         *
         * @param context
         */
        void saveState(const ExecutionContext& context);

        /**
         * @brief Reset a VM context to the last state saved by the debugger
         *
         * @param context context to reset
         */
        void resetContextToSavedState(ExecutionContext& context);

        /**
         * @brief Start the debugger shell
         *
         * @param vm
         * @param context
         */
        void run(VM& vm, ExecutionContext& context);

        inline bool isRunning() const noexcept
        {
            return m_running;
        }

    private:
        std::vector<std::unique_ptr<SavedState>> m_states;
        std::vector<std::filesystem::path> m_libenv;
        std::vector<std::string> m_symbols;
        std::vector<Value> m_constants;
        bool m_running;

        std::string m_code;  ///< Code added while inside the debugger

        /**
         * @brief Take care of compiling new code using the existing data tables
         *
         * @param code
         * @param start_page_at_offset offset to start the new pages at
         * @return std::optional<std::vector<bytecode_t>> optional set of bytecode pages if compilation succeeded
         */
        std::optional<std::vector<bytecode_t>> compile(const std::string& code, std::size_t start_page_at_offset);
    };
}

#endif  // ARK_VM_DEBUGGER_HPP
