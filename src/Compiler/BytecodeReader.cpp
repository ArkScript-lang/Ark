#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/VM/FFI.hpp>
#include <Ark/Log.hpp>
#undef abs
#include <Ark/Utils.hpp>

namespace Ark
{
    using namespace Ark::internal;

    BytecodeReader::BytecodeReader()
    {}

    void BytecodeReader::feed(const std::string& file)
    {
        std::ifstream ifs(file, std::ios::binary | std::ios::ate);
        if (!ifs.good())
            throw std::runtime_error("[BytecodeReader] Couldn't open file '" + file + "'");
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

        uint16_t major = readNumber(i); i++;
        uint16_t minor = readNumber(i); i++;
        uint16_t patch = readNumber(i); i++;
        os << "Version: " << major << "." << minor << "." << patch << "\n";

        using timestamp_t = unsigned long long;
        timestamp_t timestamp = 
            (static_cast<timestamp_t>(m_bytecode[  i]) << 56) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 48) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 40) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 32) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 24) +
            (static_cast<timestamp_t>(m_bytecode[++i]) << 16) +
            (static_cast<timestamp_t>(m_bytecode[++i]) <<  8) +
            (static_cast<timestamp_t>(m_bytecode[++i]))
            ;
        ++i;
        os << "Timestamp: " << timestamp << "\n\n";

        std::vector<std::string> symbols;
        std::vector<std::string> values;
        std::vector<std::string> plugins;

        if (b[i] == Instruction::SYM_TABLE_START)
        {
            os << "Symbols table:\n"; i++;
            uint16_t size = readNumber(i); i++;
            os << "Length: " << size << "\n";
            for (uint16_t j=0; j < size; ++j)
            {
                os << "- ";
                std::string content = "";
                while (b[i] != 0)
                    content += b[i++];
                i++;
                os << content << "\n";
                symbols.push_back(content);
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
                    os << "(Number) " << val;
                    values.push_back("(Number) " + val);
                }
                else if (type == Instruction::STRING_TYPE)
                {
                    std::string val = "";
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;
                    os << "(String) " << val;
                    values.push_back("(String) " + val);
                }
                else if (type == Instruction::FUNC_TYPE)
                {
                    uint16_t addr = readNumber(i); i++;
                    os << "(PageAddr) " << addr;
                    values.push_back("(PageAddr) " + Ark::Utils::toString(addr));
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

        if (b[i] == Instruction::PLUGIN_TABLE_START)
        {
            os << "Plugins table:\n"; i++;
            uint16_t size = readNumber(i); i++;
            os << "Length: " << size << "\n";
            for (uint16_t j=0; j < size; ++j)
            {
                os << "- ";
                std::string content = "";
                while (b[i] != 0)
                    content += b[i++];
                i++;
                os << content << "\n";
                plugins.push_back(content);
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
                    os << termcolor::cyan << (i - j) << termcolor::reset << " " << termcolor::yellow;
                    uint8_t inst = b[i]; i++;

                    if (inst == Instruction::NOP)
                        os << "NOP\n";
                    else if (inst == Instruction::LOAD_SYMBOL)
                    {
                        os << "LOAD_SYMBOL " << termcolor::green << symbols[readNumber(i) - 3] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LOAD_CONST)
                    {
                        os << "LOAD_CONST " << termcolor::magenta << values[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::POP_JUMP_IF_TRUE)
                    {
                        os << "POP_JUMP_IF_TRUE " << termcolor::red << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::STORE)
                    {
                        os << "STORE " << termcolor::green << symbols[readNumber(i) - 3] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LET)
                    {
                        os << "LET " << termcolor::green << symbols[readNumber(i) - 3] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::POP_JUMP_IF_FALSE)
                    {
                        os << "POP_JUMP_IF_FALSE " << termcolor::red << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::JUMP)
                    {
                        os << "JUMP " << termcolor::red << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::RET)
                        os << "RET\n";
                    else if (inst == Instruction::HALT)
                        os << "HALT\n";
                    else if (inst == Instruction::CALL)
                    {
                        os << "CALL " << termcolor::reset << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::NEW_ENV)
                        os << "NEW_ENV\n";
                    else if (inst == Instruction::BUILTIN)
                    {
                        os << "BUILTIN " << termcolor::reset << Ark::FFI::builtins[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::SAVE_ENV)
                        os << "SAVE_ENV\n";
                    else
                    {
                        os << "Unknown instruction: " << static_cast<int>(inst) << "\n";
                        return;
                    }

                    if (i - j == size)
                        break;
                }
            }
            os << "\n" << termcolor::reset;
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