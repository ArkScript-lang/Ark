#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/FFI.hpp>
#include <Ark/Log.hpp>

namespace Ark
{
    namespace Compiler
    {
        BytecodeReader::BytecodeReader()
        {}

        void BytecodeReader::feed(const std::string& file)
        {
            std::ifstream ifs(file, std::ios::binary | std::ios::ate);
            if (!ifs.good())
            {
                Ark::logger.error("[BytecodeReader] Couldn't open file '" + file + "'");
                exit(1);
            }
            std::ifstream::pos_type pos = ifs.tellg();
            // reserve appropriate number of bytes
            std::vector<char> temp(pos);
            ifs.seekg(0, std::ios::beg);
            ifs.read(&temp[0], pos);
            ifs.close();

            m_bytecode = bytecode_t(pos);
            for (std::size_t i=0; i < pos; ++i)
                m_bytecode[i] = static_cast<uint8_t>(temp[i]);
        }

        const bytecode_t& BytecodeReader::bytecode()
        {
            return m_bytecode;
        }

        void BytecodeReader::display()
        {
            bytecode_t b = bytecode();
            std::size_t i = 0;

            std::ostream& os = std::cout;

            if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
            {
                os << "Invalid format";
                return;
            }
            os << "Magic: ark\\0\n\n";

            if (b[i] == Instruction::SYM_TABLE_START)
            {
                os << "Symbols table:\n"; i++;
                uint16_t size = readNumber(i); i++;
                os << "Length: " << size << "\n";
                for (uint16_t j=0; j < size; ++j)
                {
                    os << "- ";
                    while (b[i] != 0)
                        os << b[i++];
                    i++;
                    os << "\n";
                }
                os << "\n";
            }

            if (b[i] == Instruction::VAL_TABLE_START)
            {
                os << "Constants table:\n"; i++;
                uint16_t size = readNumber(i); i++;
                os << "Length: " << size << "\n";
                for (uint16_t j=0; j < size; ++j)
                {
                    os << "- ";
                    uint8_t type = b[i]; i++;
                    if (type == Instruction::NUMBER_TYPE)
                    {
                        std::string val = "";
                        while (b[i] != 0)
                            val.push_back(b[i++]);
                        i++;
                        os << "(Number) 0x" << val;
                    }
                    else if (type == Instruction::STRING_TYPE)
                    {
                        std::string val = "";
                        while (b[i] != 0)
                            val.push_back(b[i++]);
                        i++;
                        os << "(String) " << val;
                    }
                    else if (type == Instruction::FUNC_TYPE)
                    {
                        uint16_t addr = readNumber(i); i++;
                        os << "(PageAddr) " << addr;
                        i++;
                    }
                    else
                    {
                        os << "Unknown value type";
                        return;
                    }
                    os << "\n";
                }
                os << "\n";
            }

            uint16_t pp = 0;

            while (b[i] == Instruction::CODE_SEGMENT_START)
            {
                os << "Code segment (PP: " << pp << ") :\n"; i++;
                uint16_t size = readNumber(i); i++;
                os << "Length: " << size << "\n";

                if (size == 0)
                    os << "NOP";
                else
                {
                    uint16_t j = i;
                    while (true)
                    {
                        os << (i - j) << " ";
                        uint8_t inst = b[i]; i++;
                        if (inst == Instruction::LOAD_SYMBOL)
                        {
                            os << "LOAD_SYMBOL (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::LOAD_CONST)
                        {
                            os << "LOAD_CONST (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::POP_JUMP_IF_TRUE)
                        {
                            os << "POP_JUMP_IF_TRUE (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::STORE)
                        {
                            os << "STORE (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::LET)
                        {
                            os << "LET (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::POP_JUMP_IF_FALSE)
                        {
                            os << "POP_JUMP_IF_FALSE (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::JUMP)
                        {
                            os << "JUMP (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::RET)
                            os << "RET\n";
                        else if (inst == Instruction::HALT)
                            os << "HALT\n";
                        else if (inst == Instruction::CALL)
                        {
                            os << "CALL (" << readNumber(i) << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::NEW_ENV)
                            os << "NEW_ENV\n";
                        else if (inst == Instruction::BUILTIN)
                        {
                            os << "BUILTIN (" << Ark::FFI::builtins[readNumber(i)] << ")\n";
                            i++;
                        }
                        else if (inst == Instruction::SAVE_ENV)
                            os << "SAVE_ENV\n";
                        else
                        {
                            os << "Unknown instruction\n";
                            return;
                        }

                        if (i - j == size)
                            break;
                    }
                }
                os << "\n";
                ++pp;
            }
        }

        uint16_t BytecodeReader::readNumber(std::size_t& i)
        {
            uint16_t x = (static_cast<uint16_t>(m_bytecode[  i]) << 8) +
                          static_cast<uint16_t>(m_bytecode[++i]);
            return x;
        }
    }
}