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

namespace Ark
{
    using namespace std::string_literals;

    template<bool debug=false, bool persist=false>
    class VM_t
    {
    public:
        VM_t();

        void feed(const std::string& filename);
        void feed(const bytecode_t& bytecode);
        void loadFunction(const std::string& name, internal::Value::ProcType function);
        void run();

    private:
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
        std::vector<std::pair<uint16_t, internal::Value>> m_locals;
        std::vector<internal::Frame> m_frames;
        std::optional<std::size_t> m_saved_frame;

        void configure();

        inline uint16_t readNumber()
        {
            return  (static_cast<uint16_t>(m_pages[m_pp][  m_ip]) << 8) +
                    (static_cast<uint16_t>(m_pages[m_pp][++m_ip])     );
        }

        inline std::optional<internal::Value*> findNearestVariable(uint16_t id)
        {
            for (auto it=m_locals.rbegin(); it != m_locals.rend(); ++it)
            {
                if (it->first == id)
                {
                    if (it->second == internal::FFI::nil)
                        return {};
                    return &(it->second);
                }
            }
            return {};
        }

        inline bool findInCurrentScope(uint16_t id)
        {
            int stop = m_frames.back().localsStart();
            for (int i=m_locals.size() - 1; i >= stop; i--)
            {
                if (m_locals[i].first == id)
                    return true;
            }
            return false;
        }

        inline internal::Value& registerVariable(uint16_t id, internal::Value&& value)
        {
            return m_locals.emplace_back(id, value).second;
        }

        inline void throwVMError(const std::string& message)
        {
            throw std::runtime_error("VMError: " + message);
        }

        internal::Value pop(int page=-1);
        void push(const internal::Value& value);
        void push(internal::Value&& value);

        // instructions
        void loadSymbol();
        void loadConst();
        void popJumpIfTrue();
        void store();
        void let();
        void popJumpIfFalse();
        void jump();
        void ret();
        void call();
        void saveEnv();
        void builtin();
        void mut();
        void del();

        void operators(uint8_t inst);
    };
}

namespace Ark
{
    #include "VM.inl"

    // debug on, persist off
    using VM_debug = VM_t<true, false>;
    // debug off, persist on
    using VM_persist = VM_t<false, true>;
    // debug on, persist on
    using VM_debug_persist = VM_t<true, true>;
    // standard VM, debug off, persist off
    using VM = VM_t<>;
}

#endif