/**
 * @file VM.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The ArkScript virtual machine
 * @version 0.3
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef ARK_VM_VM_HPP
#define ARK_VM_VM_HPP

#include <array>
#include <vector>
#include <string>
#include <cinttypes>
#include <algorithm>
#include <optional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <mutex>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/Scope.hpp>
#include <Ark/VM/State.hpp>
#include <Ark/VM/ExecutionContext.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Platform.hpp>
#include <Ark/VM/Plugin.hpp>
#include <Ark/VM/Future.hpp>

#undef abs
#include <cmath>

namespace Ark
{
    using namespace std::string_literals;

    /**
     * @brief The ArkScript virtual machine, executing ArkScript bytecode
     * 
     */
    class ARK_API VM
    {
    public:
        /**
         * @brief Construct a new vm t object
         * 
         * @param state a reference to an ArkScript state, which can be reused for multiple VMs
         */
        explicit VM(State& state) noexcept;

        [[deprecated("Use VM(State&) instead of VM(State*)")]] explicit VM(State* state) noexcept :
            VM(*state)
        {}

        /**
         * @brief Run the bytecode held in the state
         * 
         * @return int the exit code (default to 0 if no error)
         */
        int run() noexcept;

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
        [[deprecated("Use resolve(ExecutionContext*, vector<Value>&) instead")]]
        Value resolve(const Value* val, Args&&... args);

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
         * @brief Set the User Pointer object
         * 
         * @param ptr Pointer to data NOT owned by the VM, to be used later
         */
        void setUserPointer(void* ptr) noexcept;

        /**
         * @brief Retrieves the stored pointer
         * 
         * @return void* 
         */
        void* getUserPointer() noexcept;

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

        // just a nice little trick for operator[] and for pop
        Value m_no_value = internal::Builtins::nil;
        Value m_undefined_value;

        void* m_user_pointer;  ///< needed to pass data around when binding ArkScript in a program
                               // Note: This is a non owned pointer.

        /**
         * @brief Run ArkScript bytecode inside a try catch to retrieve all the exceptions and display a stack trace if needed
         * 
         * @param context
         * @param untilFrameCount the frame count we need to reach before stopping the VM
         * @return int the exit code
         */
        int safeRun(internal::ExecutionContext& context, std::size_t untilFrameCount = 0);

        /**
         * @brief Initialize the VM according to the parameters
         * 
         */
        void init() noexcept;

        /**
         * @brief Read a 2 bytes number from the current bytecode page, starting at the current instruction
         * @details Modify the instruction pointer to point on the instruction right after the number.
         * 
         * @param context
         * @return uint16_t 
         */
        inline uint16_t readNumber(internal::ExecutionContext& context);

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
         * @param val 
         * @param context 
         */
        inline void push(const Value& val, internal::ExecutionContext& context);

        /**
         * @brief Push a value on the stack
         * 
         * @param val 
         * @param context 
         */
        inline void push(Value&& val, internal::ExecutionContext& context);

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

        inline void createNewScope(internal::ExecutionContext& context) noexcept;

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
         * @param message 
         */
        void throwVMError(const std::string& message);

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
         * @param argc_ number of arguments already sent, default to -1 if it needs to search for them by itself
         */
        inline void call(internal::ExecutionContext& context, int16_t argc_ = -1);
    };

#include "inline/VM.inl"

    /// ArkScript Nil value
    const Value Nil = Value(ValueType::Nil);
    /// ArkScript False value
    const Value False = Value(ValueType::False);
    /// ArkScript True value
    const Value True = Value(ValueType::True);
}

#endif
