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

#undef abs
#include <cmath>

namespace Ark
{
    using namespace std::string_literals;

    /**
     * @brief The ArkScript virtual machine, executing ArkScript bytecode
     * 
     * @tparam debug if we need to generate a VM in a debug state or not
     */
    template<bool debug>
    class VM_t
    {
    public:
        /**
         * @brief Construct a new vm t object
         * 
         * @param state a pointer to an ArkScript state, which can be reused for multiple VMs
         */
        VM_t(State* state);

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
        internal::Value call(const std::string& name, Args&&... args)
        {
            using namespace Ark::internal;

            // reset ip and pp
            m_ip = 0;
            m_pp = 0;

            // find id of function
            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name);
            if (it == m_state->m_symbols.end())
                throwVMError("Couldn't find symbol with name " + name);

            // convert and push arguments in reverse order
            std::vector<Value> fnargs { { args... } };
            for (auto it2=fnargs.rbegin(), it_end=fnargs.rend(); it2 != it_end; ++it2)
                push(*it2);
            
            // find function object and push it if it's a pageaddr/closure
            uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
            auto var = findNearestVariable(id);
            if (var != nullptr)
            {
                if (var->m_type != ValueType::PageAddr && var->m_type != ValueType::Closure)
                    throwVMError("Symbol " + name + " isn't a function");

                push(*var);
                m_last_sym_loaded = id;
            }
            else
                throwVMError("Couldn't load symbol with name " + name);

            std::size_t frames_count = m_frames.size();
            // call it
            call(static_cast<int16_t>(sizeof...(Args)));
            // reset instruction pointer, otherwise the safeRun method will start at ip = -1
            // without doing m_ip++ as intended (done right after the call() in the loop, but here
            // we start outside this loop)
            m_ip = 0;

            // run until the function returns
            safeRun(/* untilFrameCount */ frames_count);

            // get result
            if (m_frames.back().stackSize() != 0)
                return *pop();
            else
                return Builtins::nil;
        }

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
        void init();

        /**
         * @brief Read a 2 bytes big endian number from the bytecode
         * 
         * @return uint16_t 
         */
        inline uint16_t readNumber()
        {
            auto x = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]) << 8); ++m_ip;
            auto y = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip])     );
            return x + y;
        }

        // locals related

        /**
         * @brief Register a variable in a given page
         * 
         * @tparam pp=-1 the page number
         * @param id the symbol id of the value
         * @param value the value itself
         * @return internal::Value& 
         */
        template <int pp=-1>
        inline internal::Value& registerVariable(uint16_t id, internal::Value&& value)
        {
            if constexpr (pp == -1)
                return (*m_locals.back())[id] = value;
            return (*m_locals[pp])[id] = value;
        }

        /**
         * @brief Register a variable in a given page
         * 
         * @tparam pp=-1 the page number
         * @param id the symbol id of the value
         * @param value the value itself
         * @return internal::Value& 
         */
        template <int pp=-1>
        inline internal::Value& registerVariable(uint16_t id, const internal::Value& value)
        {
            if constexpr (pp == -1)
                return (*m_locals.back())[id] = value;
            return (*m_locals[pp])[id] = value;
        }

        // could be optimized
        /**
         * @brief Find the nearest variable of a given id
         * 
         * @param id the id to find
         * @return internal::Value* 
         */
        inline internal::Value* findNearestVariable(uint16_t id)
        {
            for (auto it=m_locals.rbegin(), it_end=m_locals.rend(); it != it_end; ++it)
            {
                if ((**it)[id].m_type != internal::ValueType::Undefined)
                    return &(**it)[id];
            }
            return nullptr;
        }

        /**
         * @brief Find the nearest variable id with a given value
         * 
         * Only used to display the call stack traceback
         * 
         * @param value the value to search for
         * @return uint16_t 
         */
        inline uint16_t findNearestVariableIdWithValue(internal::Value&& value)
        {
            for (auto it=m_locals.rbegin(), it_end=m_locals.rend(); it != it_end; ++it)
            {
                for (auto sub=(*it)->begin(), sub_end=(*it)->end(); sub != sub_end; ++sub)
                {
                    if (*sub == value)
                        return static_cast<uint16_t>(std::distance((*it)->begin(), sub));
                }
            }
            // oversized by one: didn't find anything
            return static_cast<uint16_t>(m_state->m_symbols.size());
        }

        /**
         * @brief Get a variable in a scope from its id
         * 
         * @tparam pp=-1 the page number
         * @param id the id of the variable
         * @return internal::Value& 
         */
        template<int pp=-1>
        inline internal::Value& getVariableInScope(uint16_t id)
        {
            if constexpr (pp == -1)
                return (*m_locals.back())[id];
            return (*m_locals[pp])[id];
        }

        /**
         * @brief Destroy the current frame and get back to the previous one, resuming execution
         * 
         * Doing the job nobody wants to do: cleaning after everyone has finished to play.
         * This is a sort of primitive garbage collector
         * 
         */
        inline void returnFromFuncCall()
        {
            // remove frame
            m_frames.pop_back();
            uint8_t del_counter = m_frames.back().scopeCountToDelete();

            // high cpu cost because destroying variants cost
            m_locals.pop_back();

            while (del_counter != 0)
            {
                m_locals.pop_back();
                del_counter--;
            }

            m_frames.back().resetScopeCountToDelete();

            // stop the executing if we reach the wanted frame count
            if (m_frames.size() == m_until_frame_count)
                m_running = false;
        }

        /**
         * @brief Create a new Scope object which will become the current one
         * 
         */
        inline void createNewScope()
        {
            // high cpu cost because we are creating a lot of variants
            m_locals.emplace_back(
                std::make_shared<std::vector<internal::Value>>(
                    m_state->m_symbols.size(), internal::ValueType::Undefined
                )
            );
        }

        // error handling

        inline void throwVMError(const std::string& message)
        {
            throw std::runtime_error("VMError: " + message);
        }

        inline void backtrace()
        {
            std::cerr << termcolor::reset << "At IP: " << (m_ip != -1 ? m_ip : 0) << ", PP: " << m_pp << "\n";

            if (m_frames.size() > 1)
            {
                // display call stack trace
                for (auto&& it=m_frames.rbegin(), it_end=m_frames.rend(); it != it_end; ++it)
                {
                    std::cerr << "[" << termcolor::cyan << std::distance(it, m_frames.rend()) << termcolor::reset << "] ";
                    if (it->currentPageAddr() != 0)
                    {
                        uint16_t id = findNearestVariableIdWithValue(
                            Value(static_cast<PageAddr_t>(it->currentPageAddr()))
                        );
                        
                        std::cerr << "In function `" << termcolor::green << m_state->m_symbols[id] << termcolor::reset << "'\n";
                    }
                    else
                        std::cerr << "In global scope\n";

                    if (std::distance(m_frames.rbegin(), it) > 7)
                    {
                        std::cerr << "...\n";
                        break;
                    }
                }

                // if persistance is on, clear frames to keep only the global one
                if (m_state->m_options & FeaturePersist)
                    m_frames.erase(m_frames.begin() + 1, m_frames.end());
            }
        }

        // stack management

        /**
         * @brief Pop a value from the stack of a given frame
         * 
         * @param page the page number
         * @return internal::Value* 
         */
        inline internal::Value* pop(int page=-1);

        /**
         * @brief Push a value to the stack of the current frame
         * 
         * @param value 
         */
        inline void push(const internal::Value& value);

        /**
         * @brief Push a value to the stack of the current frame
         * 
         * @param value 
         */
        inline void push(internal::Value&& value);

        /**
         * @brief Function called when the CALL instruction is met in the bytecode
         * 
         * @param argc_ number of arguments already sent, default to -1 if it needs to search for them by itself
         */
        inline void call(int16_t argc_=-1);

        // function calling from plugins

        /**
         * @brief Resolving a function call (called by the resolve method of a Value)
         * 
         * @tparam Args 
         * @param val the ArkScript function object
         * @param args C++ argument list
         * @return internal::Value 
         */
        template <typename... Args>
        internal::Value resolve(const internal::Value* val, Args&&... args)
        {
            using namespace Ark::internal;

            if (!val->isFunction())
                throw Ark::TypeError("Value::resolve couldn't resolve a non-function");

            int ip = m_ip;
            std::size_t pp = m_pp;

            // convert and push arguments in reverse order
            std::vector<Value> fnargs { { args... } };
            for (auto it=fnargs.rbegin(), it_end=fnargs.rend(); it != it_end; ++it)
                push(*it);
            // push function
            push(*val);

            std::size_t frames_count = m_frames.size();
            // call it
            call(static_cast<int16_t>(sizeof...(Args)));
            // reset instruction pointer, otherwise the safeRun method will start at ip = -1
            // without doing m_ip++ as intended (done right after the call() in the loop, but here
            // we start outside this loop)
            m_ip = 0;

            // run until the function returns
            safeRun(/* untilFrameCount */ frames_count);

            // restore VM state
            m_ip = ip;
            m_pp = pp;

            // get result
            if (m_frames.back().stackSize() != 0)
                return *pop();
            else
                return Builtins::nil;
        }
    };

    #include "inline/VM.inl"

    namespace internal
    {
        #include "inline/Value_VM.inl"
    }

    /// VM with debug on
    using VM_debug = VM_t<true>;

    /// standard VM, debug off
    using VM = VM_t<false>;

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