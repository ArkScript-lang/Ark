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

#include <utility>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <functional>
#include <filesystem>

#include <Ark/Compiler/Common.hpp>
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

    struct CompiledPrompt
    {
        std::vector<bytecode_t> pages;
        std::vector<std::string> symbols;
        std::vector<Value> constants;
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
        Debugger(const ExecutionContext& context, const std::vector<std::filesystem::path>& libenv, const std::vector<std::string>& symbols, const std::vector<Value>& constants);

        /**
         * @brief Create a new Debugger object that will use lines from a file as prompts, instead of waiting for user inputs
         *
         * @param libenv
         * @param path_to_prompt_file
         * @param os output stream
         * @param symbols symbols table of the VM
         * @param constants constants table of the VM
         */
        Debugger(const std::vector<std::filesystem::path>& libenv, const std::string& path_to_prompt_file, std::ostream& os, const std::vector<std::string>& symbols, const std::vector<Value>& constants);

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
         * @param from_breakpoint true if the debugger is being invoked from a breakpoint
         */
        void run(VM& vm, ExecutionContext& context, bool from_breakpoint);

        void registerInstruction(uint32_t word) noexcept;

        [[nodiscard]] ARK_ALWAYS_INLINE bool isRunning() const noexcept
        {
            return m_running;
        }

        [[nodiscard]] ARK_ALWAYS_INLINE bool shouldQuitVM() const noexcept
        {
            return m_quit_vm;
        }

    private:
        struct Command;
        struct CommandArgs
        {
            VM* vm_ptr;
            ExecutionContext* ctx_ptr;
            std::size_t ip, pp;
            const Command& me;
        };

        struct StartsWith
        {
            std::string prefix;
        };

        struct Command
        {
            using Action_t = std::function<bool(const std::string&, const CommandArgs&)>;
            using Args_t = std::vector<std::pair<std::string, std::string>>;

            bool is_exact;
            std::vector<std::string> names;
            Args_t args;
            std::string description;
            Action_t action;

            Command(std::string name, std::string desc, Action_t&& do_this) :
                is_exact(true), names({ std::move(name) }), description(std::move(desc)), action(do_this)
            {}

            Command(const std::initializer_list<std::string> list_of_names, std::string desc, Action_t&& do_this) :
                is_exact(true), names(list_of_names), description(std::move(desc)), action(do_this)
            {}

            Command(StartsWith start, std::vector<std::pair<std::string, std::string>> arguments, std::string desc, Action_t&& do_this) :
                is_exact(false), names({ std::move(start.prefix) }), args(std::move(arguments)), description(std::move(desc)), action(std::move(do_this))
            {}

            [[nodiscard]] std::optional<Args_t> getArgs(const std::string& line, std::ostream& os) const;

            [[nodiscard]] std::optional<std::size_t> argAsCount(const std::string& line, std::size_t idx, std::ostream& os) const;
        };

        std::vector<Command> m_commands;

        std::vector<std::unique_ptr<SavedState>> m_states;
        std::vector<std::filesystem::path> m_libenv;
        std::vector<std::string> m_symbols;
        std::vector<Value> m_constants;
        bool m_running { false };
        bool m_quit_vm { false };

        std::vector<uint32_t> m_previous_insts;

        std::ostream& m_os;
        bool m_colorize;
        std::unique_ptr<std::istream> m_prompt_stream;
        std::string m_code;  ///< Code added while inside the debugger
        std::size_t m_line_count { 0 };

        void initCommands();
        [[nodiscard]] std::optional<Command> matchCommand(const std::string& line) const;

        void showContext(const VM& vm, const ExecutionContext& context) const;
        void showStack(VM& vm, const ExecutionContext& context, std::size_t count) const;
        void showLocals(VM& vm, ExecutionContext& context, std::size_t count) const;
        void showScopes(VM& vm, ExecutionContext& context, std::size_t count) const;
        void showPreviousInstructions(const VM& vm, std::size_t count) const;

        void showLocals(const ScopeView& scope, VM& vm, std::optional<std::size_t> limit = std::nullopt) const;

        std::optional<std::string> prompt(std::size_t ip, std::size_t pp, VM& vm, ExecutionContext& context);

        /**
         * @brief Take care of compiling new code using the existing data tables
         *
         * @param code
         * @param start_page_at_offset offset to start the new pages at
         * @return std::optional<CompiledPrompt> optional set of bytecode pages, symbols and constants if compilation succeeded
         */
        [[nodiscard]] std::optional<CompiledPrompt> compile(const std::string& code, std::size_t start_page_at_offset) const;
    };
}

#endif  // ARK_VM_DEBUGGER_HPP
