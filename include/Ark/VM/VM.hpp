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
#include <Ark/VM/FFI.hpp>
#include <Ark/Log.hpp>

#undef abs
#include <cmath>

namespace Ark
{
    using namespace std::string_literals;

    template<bool debug>
    class VM_t
    {
    public:
        VM_t(State* state);

        int run();

        internal::Value& operator[](const std::string& name);

        template <typename... Args>
        internal::Value call(const std::string& name, Args&&... args)
        {
            using namespace Ark::internal;

            // reset ip and pp
            m_ip = m_pp = 0;

            // find id of function
            auto it = std::find(m_state->m_symbols.begin(), m_state->m_symbols.end(), name);
            if (it == m_state->m_symbols.end())
            {
                if constexpr (debug)
                    throwVMError("Couldn't find symbol with name " + name);
            }

            // convert and push arguments in reverse order
            std::vector<Value> fnargs { args... };
            for (auto it2=fnargs.rbegin(); it2 != fnargs.rend(); ++it2)
                push(*it2);
            
            // find function object and push it if it's a pageaddr/closure
            uint16_t id = static_cast<uint16_t>(std::distance(m_state->m_symbols.begin(), it));
            auto var = findNearestVariable(id);
            if (var != nullptr)
            {
                if (var->valueType() != ValueType::PageAddr && var->valueType() != ValueType::Closure)
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
                return pop();
            else
                return FFI::nil;
        }

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
        internal::Value m__no_value = internal::FFI::nil;

        void configure();
        int safeRun(std::size_t untilFrameCount=0);
        void init();

        inline uint16_t readNumber()
        {
            auto x = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip]) << 8); ++m_ip;
            auto y = (static_cast<uint16_t>(m_state->m_pages[m_pp][m_ip])     );
            return x + y;
        }

        // locals related

        template <int pp=-1>
        inline internal::Value& registerVariable(uint16_t id, internal::Value&& value)
        {
            if constexpr (pp == -1)
                return (*m_locals.back())[id] = value;
            return (*m_locals[pp])[id] = value;
        }

        template <int pp=-1>
        inline internal::Value& registerVariable(uint16_t id, const internal::Value& value)
        {
            if constexpr (pp == -1)
                return (*m_locals.back())[id] = value;
            return (*m_locals[pp])[id] = value;
        }

        inline internal::Value* findNearestVariable(uint16_t id)
        {
            for (auto it=m_locals.rbegin(); it != m_locals.rend(); ++it)
            {
                if ((**it)[id] != internal::FFI::undefined)
                    return &(**it)[id];
            }
            return nullptr;
        }

        inline uint16_t findNearestVariableIdWithValue(internal::Value&& value)
        {
            for (auto it=m_locals.rbegin(); it != m_locals.rend(); ++it)
            {
                for (auto sub=(*it)->begin(); sub != (*it)->end(); ++sub)
                {
                    if (*sub == value)
                        return static_cast<uint16_t>(std::distance((*it)->begin(), sub));
                }
            }
            // oversized by one: didn't find anything
            return static_cast<uint16_t>(m_state->m_symbols.size());
        }

        template<int pp=-1>
        inline internal::Value& getVariableInScope(uint16_t id)
        {
            if constexpr (pp == -1)
                return (*m_locals.back())[id];
            return (*m_locals[pp])[id];
        }

        inline void returnFromFuncCall()
        {
            // remove frame
            m_frames.pop_back();
            uint8_t del_counter = m_frames.back().scopeCountToDelete();
            m_locals.pop_back();
            
            while (del_counter != 0)
            {
                m_locals.pop_back();
                del_counter--;
            }

            m_frames.back().resetScopeCountToDelete();

            if (m_frames.size() == m_until_frame_count)
                m_running = false;
        }

        inline void createNewScope()
        {
            m_locals.emplace_back(
                std::make_shared<std::vector<internal::Value>>(
                    m_state->m_symbols.size(), internal::FFI::undefined
                )
            );
        }

        // error handling

        inline void throwVMError(const std::string& message)
        {
            throw std::runtime_error("VMError: " + message);
        }

        // stack management

        inline internal::Value&& pop(int page=-1);
        inline void push(const internal::Value& value);
        inline void push(internal::Value&& value);

        // instructions
        inline void loadSymbol();
        inline void loadConst();
        inline void popJumpIfTrue();
        inline void store();
        inline void let();
        inline void popJumpIfFalse();
        inline void jump();
        inline void ret();
        inline void call(int16_t argc_=-1);
        inline void capture();
        inline void builtin();
        inline void mut();
        inline void del();
        inline void saveEnv();
        inline void getField();

        inline void operators(uint8_t inst);
    };
}

namespace Ark
{
    #include "VM.inl"

    // debug on
    using VM_debug = VM_t<true>;
    // standard VM, debug off
    using VM = VM_t<false>;

    // aliases
    using Value = internal::Value;
    using ValueType = internal::ValueType;
    const Value Nil = Value(internal::NFT::Nil);
    const Value False = Value(internal::NFT::False);
    const Value True = Value(internal::NFT::True);
}

#endif