#ifndef ark_compiler_bytecodereader
#define ark_compiler_bytecodereader

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cinttypes>

namespace Ark
{
    namespace Compiler
    {
        using bytecode_t = std::vector<uint8_t>;

        class BytecodeReader
        {
        public:
            BytecodeReader();

            void feed(const std::string& file);
            const bytecode_t& bytecode();

            void display();
        
        private:
            bytecode_t m_bytecode;

            uint16_t readNumber(std::size_t& i);
        };
    }
}

#endif