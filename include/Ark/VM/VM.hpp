/**
 * @file VM.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The ArkScript virtual machine
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef ark_vm
#define ark_vm

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
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Log.hpp>
#include <Ark/Config.hpp>
#include <Ark/VM/Plugin.hpp>

#define ARK_STACK_SIZE 8192

#undef abs
#include <cmath>

namespace Ark
{
    using namespace std::string_literals;

    /**
     * @brief The ArkScript virtual machine, executing ArkScript bytecode
     * 
     */
    class ARK_API_EXPORT VM
    {
    public:
        /**
         * @brief Construct a new vm t object
         * 
         * @param state a pointer to an ArkScript state, which can be reused for multiple VMs
         */
        explicit VM(State* state) noexcept;

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
         * @return internal::Value& 
         */
        internal::Value& operator[](const std::string& name) noexcept;

        /**
         * @brief Call a function from ArkScript, by giving it arguments
         * 
         * @tparam Args 
         * @param name the function name in the ArkScript code
         * @param args C++ argument list, converted to internal representation
         * @return internal::Value 
         */
        template <typename... Args>
        internal::Value call(const std::string& name, Args&&... args);

        // ================================================
        //         function calling from plugins
        // ================================================

        /**
         * @brief Resolving a function call (called by plugins)
         * 
         * @tparam Args 
         * @param val the ArkScript function object
         * @param args C++ argument list
         * @return internal::Value 
         */
        template <typename... Args>
        internal::Value resolve(const internal::Value* val, Args&&... args);

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

        friend class internal::Value;
        friend class Repl;

    private:
        State* m_state;

        int m_exitCode;     ///< VM exit code, defaults to 0. Can be changed through `sys:exit`
        int m_ip;           ///< instruction pointer
        std::size_t m_pp;   ///< page pointer
        uint16_t m_sp;      ///< stack pointer
        uint16_t m_fc;      ///< current frames count
        bool m_running;
        uint16_t m_last_sym_loaded;
        std::size_t m_until_frame_count;
        std::mutex m_mutex;

        // related to the execution
        std::array<internal::Value, ARK_STACK_SIZE> m_stack;
        std::vector<uint8_t> m_scope_count_to_delete;
        std::optional<internal::Scope_t> m_saved_scope;
        std::vector<internal::Scope_t> m_locals;
        std::vector<std::shared_ptr<internal::SharedLibrary>> m_shared_lib_objects;

        // just a nice little trick for operator[] and for pop
        internal::Value m__no_value = internal::Builtins::nil;

        void* m_user_pointer; ///< needed to pass data around when binding ArkScript in a program

        /**
         * @brief Run ArkScript bytecode inside a try catch to retrieve all the exceptions and display a stack trace if needed
         * 
         * @param untilFrameCount the frame count we need to reach before stopping the VM
         * @return int the exit code
         */
        int safeRun(std::size_t untilFrameCount=0);

        /**
         * @brief Initialize the VM according to the parameters
         * 
         */
        void init() noexcept;

        // ================================================
        //                 stack related
        // ================================================

        /**
         * @brief Pop a value from the stack
         * 
         * @return internal::Value* 
         */
        inline internal::Value* pop();

        /**
         * @brief Push a value on the stack
         * 
         * @param val 
         */
        inline void push(const internal::Value& val);

        /**
         * @brief Push a value on the stack
         * 
         * @param val 
         */
        inline void push(internal::Value&& val);

        /**
         * @brief Push a value on the stack
         * 
         * @param val 
         */
        inline void push(internal::Value* valptr);

        /**
         * @brief Pop a value from the stack and resolve it if possible, then return it
         * 
         * @return internal::Value* 
         */
        inline internal::Value* popAndResolveAsPtr();

        // ================================================
        //                locals related
        // ================================================

        /**
         * @brief Find the nearest variable of a given id
         * 
         * @param id the id to find
         * @return internal::Value* 
         */
        inline internal::Value* findNearestVariable(uint16_t id) noexcept;

        /**
         * @brief Destroy the current frame and get back to the previous one, resuming execution
         * 
         * Doing the job nobody wants to do: cleaning after everyone has finished to play.
         * This is a sort of primitive garbage collector
         * 
         */
        inline void returnFromFuncCall();

        /**
         * @brief Load a plugin from a constant id
         * 
         * @param id Id of the constant
         */
        void loadPlugin(uint16_t id);

        // ================================================
        //                  error handling
        // ================================================

        /**
         * @brief Find the nearest variable id with a given value
         * 
         * Only used to display the call stack traceback
         * 
         * @param value the value to search for
         * @return uint16_t 
         */
        uint16_t findNearestVariableIdWithValue(internal::Value&& value) noexcept;

        /**
         * @brief Throw a VM error message
         * 
         * @param message 
         */
        void throwVMError(const std::string& message);

        /**
         * @brief Display a backtrace when the VM encounter an exception
         * 
         */
        void backtrace() noexcept;

        /**
         * @brief Function called when the CALL instruction is met in the bytecode
         * 
         * @param argc_ number of arguments already sent, default to -1 if it needs to search for them by itself
         */
        inline void call(int16_t argc_=-1);
    };

    #include "inline/VM.inl"

    // aliases
    using Value = internal::Value;
    using ValueType = internal::ValueType;

    /// ArkScript Nil value
    const Value Nil = Value(internal::ValueType::Nil);
    /// ArkScript False value
    const Value False = Value(internal::ValueType::False);
    /// ArkScript True value
    const Value True = Value(internal::ValueType::True);
}

#endif