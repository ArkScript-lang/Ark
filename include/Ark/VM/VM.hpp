#ifndef ark_vm
#define ark_vm

#include <vector>
#include <string>
#include <cinttypes>
#include <algorithm>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/Frame.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Compiler/Instructions.hpp>

namespace Ark
{
    namespace VM
    {
        using namespace Ark::Compiler;

        class VM
        {
        public:
            VM(bool debug=false);
            ~VM();

            void feed(const std::string& filename);
            void run();

            void loadFunction(const std::string& name, Value::ProcType function);
        
        private:
            bool m_debug;
            bytecode_t m_bytecode;
            // Instruction Pointer and Page Pointer
            int m_ip;
            std::size_t m_pp;
            bool m_running;

            std::vector<Value::ProcType> m_ffi;

            std::vector<std::string> m_symbols;
            std::vector<Value> m_constants;
            std::vector<bytecode_t> m_pages;

            std::vector<Frame> m_frames;
            std::unordered_map<PageAddr_t, Frame> m_saved_frames;

            void configure();
            void initFFI();

            inline uint16_t readNumber()
            {
                return (static_cast<uint16_t>(m_pages[m_pp][  m_ip]) << 8) +
                       (static_cast<uint16_t>(m_pages[m_pp][++m_ip])     );
            }

            Value pop();
            void push(const Value& value);

            // instructions
            void nop();
            void loadSymbol();
            void loadConst();
            void popJumpIfTrue();
            void store();
            void let();
            void popJumpIfFalse();
            void jump();
            void ret();
            void call();
            void newEnv();
            void builtin();
        };
    }
}

#endif