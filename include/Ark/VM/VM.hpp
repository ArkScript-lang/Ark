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
    class VM
    {
    public:
        VM(bool debug=false, bool count_fcall=false, bool persist=false);

        void feed(const std::string& filename);
        void feed(const bytecode_t& bytecode);
        void run();

        void loadFunction(const std::string& name, internal::Value::ProcType function);

    private:
        bool m_debug;
        bool m_count_fcall;
        bool m_persist;
        uint64_t m_fcalls;
        bytecode_t m_bytecode;
        // Instruction Pointer and Page Pointer
        int m_ip;
        std::size_t m_pp;
        bool m_running;
        std::string m_filename;
        uint16_t m_last_sym_loaded;

        std::vector<std::string> m_symbols;
        std::vector<internal::Value> m_constants;
        std::vector<std::string> m_plugins;
        std::vector<internal::SharedLibrary> m_shared_lib_objects;
        std::vector<bytecode_t> m_pages;

        std::vector<std::pair<uint16_t, internal::Value>> m_locals;
        std::vector<internal::Frame> m_frames;
        std::optional<std::size_t> m_saved_frame;

        void unsafeRun();
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

        inline internal::Value& registerVariable(uint16_t id, internal::Value&& value)
        {
            return m_locals.emplace_back(id, value).second;
        }

        inline internal::Frame& frontFrame() { return m_frames.front(); }
        inline internal::Frame& backFrame()  { return m_frames.back();  }
        inline internal::Frame& frameAt(std::size_t i) { return m_frames[i]; }
        inline void createNewFrame() { m_frames.emplace_back() ; }

        inline internal::Value pop(int page=-1);
        inline void push(const internal::Value& value);

        // instructions
        inline void nop();
        inline void loadSymbol();
        inline void loadConst();
        inline void popJumpIfTrue();
        inline void store();
        inline void let();
        inline void popJumpIfFalse();
        inline void jump();
        inline void ret();
        inline void call();
        inline void saveEnv();
        inline void builtin();
        inline void mut();
        inline void del();

        inline void operators(uint8_t inst);

        inline void throwVMError(const std::string& message)
        {
            throw std::runtime_error("VMError: " + message);
        }
    };
}

#endif