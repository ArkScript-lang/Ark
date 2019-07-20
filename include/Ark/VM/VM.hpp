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
#include <Ark/Compiler/Instructions.hpp>
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
        VM_t(bool persist=false);

        void feed(const std::string& filename);
        void feed(const bytecode_t& bytecode);
        void loadFunction(const std::string& name, internal::Value::ProcType function);
        void run();

    private:
        bool m_persist;
        bytecode_t m_bytecode;
        // Instruction Pointer and Page Pointer
        int m_ip;
        std::size_t m_pp;
        bool m_running;
        std::string m_filename;
        uint16_t m_last_sym_loaded;

        // related to the bytecode
        std::vector<std::string> m_symbols;
        std::vector<internal::Value> m_constants;
        std::vector<std::string> m_plugins;
        std::vector<internal::SharedLibrary> m_shared_lib_objects;
        std::vector<bytecode_t> m_pages;

        // related to the execution
        std::vector<internal::Frame> m_frames;
        std::optional<internal::Closure::Scope_t> m_saved_scope;
        std::vector<internal::Closure::Scope_t> m_locals;

        void configure();

        inline uint16_t readNumber()
        {
            auto x = (static_cast<uint16_t>(m_pages[m_pp][m_ip]) << 8); ++m_ip;
            auto y = (static_cast<uint16_t>(m_pages[m_pp][m_ip])     );
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
            const std::size_t s = m_symbols.size();
            for (auto it=m_locals.rbegin(); it != m_locals.rend(); ++it)
            {
                if ((**it)[id] != internal::FFI::undefined)
                    return &(**it)[id];
            }
            return nullptr;
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
            bool is_closure = m_frames.back().isClosure();
            m_frames.pop_back();
            m_locals.pop_back();
            if (is_closure)
            {
                // next environment is the one of the closure
                // remove it
                m_locals.pop_back();
            }
        }

        inline void createNewScope()
        {
            m_locals.emplace_back(
                std::make_shared<std::vector<internal::Value>>(
                    m_symbols.size(), internal::FFI::undefined
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
        inline void call();
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
}

#endif