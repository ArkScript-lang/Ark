/**
 * @file VM.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The ArkScript virtual machine
 * @version 2.0
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_VM_VM_HPP
#define ARK_VM_VM_HPP

#include <array>
#include <vector>
#include <string>
#include <cassert>
#include <utility>
#include <cinttypes>
#include <unordered_map>
#include <algorithm>
#include <fmt/core.h>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/VM/Value.hpp>
#include <Ark/VM/State.hpp>
#include <Ark/VM/ErrorKind.hpp>
#include <Ark/VM/ExecutionContext.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Platform.hpp>
#include <Ark/VM/Plugin.hpp>
#include <Ark/VM/Future.hpp>

namespace Ark
{
    using namespace std::string_literals;

    /**
     * @brief The ArkScript virtual machine, executing ArkScript bytecode
     *
     */
    class ARK_API VM final
    {
    public:
        /**
         * @brief Construct a new vm t object
         *
         * @param state a reference to an ArkScript state, which can be reused for multiple VMs
         */
        explicit VM(State& state) noexcept;

        /**
         * @brief Run the bytecode held in the state
         *
         * @param fail_with_exception throw if true, display a stacktrace if false
         * @return int the exit code (default to 0 if no error)
         */
        int run(bool fail_with_exception = false);

        /**
         * @brief Retrieve a value from the virtual machine, given its symbol name
         *
         * @param name the name of the variable to retrieve
         * @return Value&
         */
        Value& operator[](const std::string& name) noexcept;

        /**
         * @brief Call a function from ArkScript, by giving it arguments
         *
         * @tparam Args
         * @param name the function name in the ArkScript code
         * @param args C++ argument list, converted to internal representation
         * @return Value
         */
        template <typename... Args>
        Value call(const std::string& name, Args&&... args);

        // ================================================
        //         function calling from plugins
        // ================================================

        /**
         * @brief Resolving a function call (called by plugins and builtins)
         *
         * @tparam Args
         * @param val the ArkScript function object
         * @param args C++ argument list
         * @return Value
         */
        template <typename... Args>
        [[deprecated("Use resolve(ExecutionContext*, vector<Value>&) instead")]] Value resolve(const Value* val, Args&&... args);

        /**
         * @brief Resolves a function call (called by plugins and builtins)
         *
         * @param context the execution context to use
         * @param n the function and its arguments
         * @return Value
         */
        inline Value resolve(internal::ExecutionContext* context, std::vector<Value>& n);

        /**
         * @brief Ask the VM to exit with a given exit code
         *
         * @param code an exit code
         */
        void exit(int code) noexcept;

        /**
         * @brief Create an execution context and returns it
         * @details This method is thread-safe VM wise.
         *
         * @return internal::ExecutionContext*
         */
        internal::ExecutionContext* createAndGetContext();

        /**
         * @brief Free a given execution context
         * @details This method is thread-safe VM wise.
         *
         * @param ec
         */
        void deleteContext(internal::ExecutionContext* ec);

        /**
         * @brief Create a Future object from a function and its arguments and return a managed pointer to it
         * @details This method is thread-safe VM wise.
         *
         * @param args
         * @return internal::Future*
         */
        internal::Future* createFuture(std::vector<Value>& args);

        /**
         * @brief Free a given future
         * @details This method is thread-safe VM wise.
         *
         * @param f
         */
        void deleteFuture(internal::Future* f);

        /**
         * @brief Used by the REPL to force reload all the plugins and their bound methods
         *
         * @return true on success
         * @return false if one or more plugins couldn't be reloaded
         */
        [[nodiscard]] bool forceReloadPlugins() const;

        friend class Value;
        friend class internal::Closure;
        friend class Repl;

    private:
        State& m_state;
        std::vector<std::unique_ptr<internal::ExecutionContext>> m_execution_contexts;
        int m_exit_code;  ///< VM exit code, defaults to 0. Can be changed through `sys:exit`
        bool m_running;
        std::mutex m_mutex;
        std::vector<std::shared_ptr<internal::SharedLibrary>> m_shared_lib_objects;
        std::vector<std::unique_ptr<internal::Future>> m_futures;  ///< Storing the promises while we are resolving them

        // a little trick for operator[] and for pop
        Value m_no_value = internal::Builtins::nil;
        Value m_undefined_value;

        /**
         * @brief Run ArkScript bytecode inside a try catch to retrieve all the exceptions and display a stack trace if needed
         *
         * @param context
         * @param untilFrameCount the frame count we need to reach before stopping the VM
         * @param fail_with_exception throw if true, display a stacktrace if false
         * @return int the exit code
         */
        int safeRun(internal::ExecutionContext& context, std::size_t untilFrameCount = 0, bool fail_with_exception = false);

        /**
         * @brief Initialize the VM according to the parameters
         *
         */
        void init() noexcept;

        // ================================================
        //               instruction helpers
        // ================================================

        inline Value* loadSymbol(uint16_t id, internal::ExecutionContext& context);
        inline Value* loadConstAsPtr(uint16_t id) const;
        inline void store(uint16_t id, const Value* val, internal::ExecutionContext& context);
        inline void setVal(uint16_t id, const Value* val, internal::ExecutionContext& context);

        // ================================================
        //                 stack related
        // ================================================

        /**
         * @brief Pop a value from the stack
         *
         * @param context
         * @return Value*
         */
        inline Value* pop(internal::ExecutionContext& context);

        /**
         * @brief Push a value on the stack
         *
         * @param value
         * @param context
         */
        inline void push(const Value& value, internal::ExecutionContext& context);

        /**
         * @brief Push a value on the stack
         *
         * @param value
         * @param context
         */
        inline void push(Value&& value, internal::ExecutionContext& context);

        /**
         * @brief Push a value on the stack as a reference
         *
         * @param valptr
         * @param context
         */
        inline void push(Value* valptr, internal::ExecutionContext& context);

        /**
         * @brief Pop a value from the stack and resolve it if possible, then return it
         *
         * @param context
         * @return Value*
         */
        inline Value* popAndResolveAsPtr(internal::ExecutionContext& context);

        /**
         * @brief Move stack values around and invert them
         * @details values:     1,  2, 3, _, _
         *          wanted:    pp, ip, 3, 2, 1
         *          positions:  0,  1, 2, 3, 4
         *
         * @param argc number of arguments to swap around
         * @param context
         */
        inline void swapStackForFunCall(uint16_t argc, internal::ExecutionContext& context);

        // ================================================
        //                locals related
        // ================================================

        /**
         * @brief Find the nearest variable of a given id
         *
         * @param id the id to find
         * @param context
         * @return Value*
         */
        inline Value* findNearestVariable(uint16_t id, internal::ExecutionContext& context) noexcept;

        /**
         * @brief Destroy the current frame and get back to the previous one, resuming execution
         *
         * Doing the job nobody wants to do: cleaning after everyone has finished to play.
         * This is a sort of primitive garbage collector
         *
         * @param context
         */
        inline void returnFromFuncCall(internal::ExecutionContext& context);

        /**
         * @brief Load a plugin from a constant id
         *
         * @param id Id of the constant
         * @param context
         */
        void loadPlugin(uint16_t id, internal::ExecutionContext& context);

        // ================================================
        //                  error handling
        // ================================================

        /**
         * @brief Find the nearest variable id with a given value
         *
         * Only used to display the call stack traceback
         *
         * @param value the value to search for
         * @param context
         * @return uint16_t
         */
        uint16_t findNearestVariableIdWithValue(const Value& value, internal::ExecutionContext& context) const noexcept;

        /**
         * @brief Throw a VM error message
         *
         * @param kind type of VM error
         * @param message
         */
        static void throwVMError(internal::ErrorKind kind, const std::string& message);

        /**
         * @brief Display a backtrace when the VM encounter an exception
         *
         * @param context
         */
        void backtrace(internal::ExecutionContext& context) noexcept;

        /**
         * @brief Function called when the CALL instruction is met in the bytecode
         *
         * @param context
         * @param argc number of arguments already sent
         */
        inline void call(internal::ExecutionContext& context, uint16_t argc);

        /**
         * @brief Builtin called when the CALL_BUILTIN instruction is met in the bytecode
         *
         * @param context
         * @param builtin the builtin to call
         * @param argc number of arguments already sent
         */
        inline void callBuiltin(internal::ExecutionContext& context, const Value& builtin, uint16_t argc);
    };

#include "VM.inl"

    /// ArkScript Nil value
    const auto Nil = Value(ValueType::Nil);
    /// ArkScript False value
    const auto False = Value(ValueType::False);
    /// ArkScript True value
    const auto True = Value(ValueType::True);
}

#endif
