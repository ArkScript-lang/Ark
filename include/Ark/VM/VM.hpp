#ifndef ark_vm
#define ark_vm

#include <vector>
#include <string>
#include <cinttypes>
#include <algorithm>
#include <optional>
#include <memory>
#include <unordered_map>
#include <utility>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/Frame.hpp>
#include <Ark/VM/State.hpp>
#include <Ark/VM/Plugin.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Log.hpp>
#include <Ark/Config.hpp>

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
        VM(State* state);

        /**
         * @brief Run the bytecode held in the state
         * 
         * @return int the exit code (default to 0 if no error)
         */
        int run();

        /**
         * @brief Retrieve a value from the virtual machine, given its symbol name
         * 
         * @param name the name of the variable to retrieve
         * @return internal::Value& 
         */
        internal::Value& operator[](const std::string& name);

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

        // function calling from plugins

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

        friend class internal::Value;

    private:
        State* m_state;

        int m_ip;           // instruction pointer
        std::size_t m_pp;   // page pointer
        bool m_running;
        uint16_t m_last_sym_loaded;
        std::size_t m_until_frame_count;

        // related to the execution
        std::vector<internal::Frame> m_frames;
        std::optional<internal::Scope_t> m_saved_scope;
        std::vector<internal::Scope_t> m_locals;
        std::vector<internal::SharedLibrary> m_shared_lib_objects;

        // just a nice little trick for operator[]
        internal::Value m__no_value = internal::Builtins::nil;

        void configure();

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
        void init();

        // locals related

        /**
         * @brief Find the nearest variable of a given id
         * 
         * @param id the id to find
         * @return internal::Value* 
         */
        inline internal::Value* findNearestVariable(uint16_t id);

        /**
         * @brief Destroy the current frame and get back to the previous one, resuming execution
         * 
         * Doing the job nobody wants to do: cleaning after everyone has finished to play.
         * This is a sort of primitive garbage collector
         * 
         */
        inline void returnFromFuncCall();

        // error handling

        /**
         * @brief Find the nearest variable id with a given value
         * 
         * Only used to display the call stack traceback
         * 
         * @param value the value to search for
         * @return uint16_t 
         */
        uint16_t findNearestVariableIdWithValue(internal::Value&& value);

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
        void backtrace();

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