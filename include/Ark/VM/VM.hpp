#ifndef ark_vm
#define ark_vm

#include <vector>
#include <string>
#include <cinttypes>

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
        
        private:
            bool m_debug;
            bytecode_t m_bytecode;
            // Instruction Pointer and Page Pointer
            std::size_t m_ip, m_pp;

            std::vector<std::string> m_symbols;
            std::vector<Value> m_constants;
            std::vector<bytecode_t> m_pages;

            std::vector<Frame> m_frames;

            void configure();
        };
    }
}

#endif