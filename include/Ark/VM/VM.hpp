#ifndef ark_vm
#define ark_vm

#include <vector>
#include <string>
#include <cinttypes>

#include <Ark/VM/Value.hpp>
#include <Ark/VM/Frame.hpp>
#include <Ark/Compiler/BytecodeReader.hpp>
#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Lang/Environment.hpp>

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
        
        private:
            bool m_debug;
            bytecode_t m_bytecode;
            // Instruction Pointer and Page Pointer
            std::size_t m_ip, m_pp;
            bool m_running;

            Ark::Lang::Environment m_ffi;

            std::vector<std::string> m_symbols;
            std::vector<Value> m_constants;
            std::vector<bytecode_t> m_pages;

            std::vector<Frame> m_frames;

            void configure();
            inline uint16_t readNumber()
            {
                return (static_cast<uint16_t>(m_pages[m_pp][  m_ip]) << 8) +
                       (static_cast<uint16_t>(m_pages[m_pp][++m_ip])     );
            }

            inline bool isNumber(const Value& value) { return std::holds_alternative<HugeNumber>(value); }
            inline bool isString(const Value& value) { return std::holds_alternative<std::string>(value); }
            inline bool isPageAddr(const Value& value) { return std::holds_alternative<PageAddr_t>(value); }
            inline bool isNFT(const Value& value) { return std::holds_alternative<NFT>(value); }
            inline bool isProc(const Value& value) { return std::holds_alternative<Ark::Lang::Node::ProcType>(value); }

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