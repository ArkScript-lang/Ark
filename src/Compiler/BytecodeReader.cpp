#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Log.hpp>
#undef abs
#include <Ark/Utils.hpp>

namespace Ark
{
    using namespace Ark::internal;

    void BytecodeReader::feed(const std::string& file)
    {
        std::ifstream ifs(file, std::ios::binary | std::ios::ate);
        if (!ifs.good())
            throw std::runtime_error("[BytecodeReader] Couldn't open file '" + file + "'");
        std::size_t pos = ifs.tellg();
        // reserve appropriate number of bytes
        std::vector<char> temp(pos);
        ifs.seekg(0, std::ios::beg);
        ifs.read(&temp[0], pos);
        ifs.close();

        m_bytecode = bytecode_t(pos);
        for (std::size_t i=0; i < pos; ++i)
            m_bytecode[i] = static_cast<uint8_t>(temp[i]);
    }

    const bytecode_t& BytecodeReader::bytecode() noexcept
    {
        return m_bytecode;
    }

    unsigned long long BytecodeReader::timestamp()
    {
        bytecode_t b = bytecode();
        std::size_t i = 0;

        // we want to see a 'ark\0' header
        if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
            return 0;

        uint16_t major = readNumber(i); i++;
        uint16_t minor = readNumber(i); i++;
        uint16_t patch = readNumber(i); i++;

        // reading the timestamp in big endian
        using timestamp_t = unsigned long long;
        timestamp_t timestamp = 0;
        auto aa = (static_cast<timestamp_t>(m_bytecode[  i]) << 56),
             ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
             ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
             da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
             ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
             fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
             ga = (static_cast<timestamp_t>(m_bytecode[++i]) <<  8),
             ha = (static_cast<timestamp_t>(m_bytecode[++i]));
        i++;
        timestamp = aa + ba + ca + da + ea + fa + ga + ha;

        return timestamp;
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
        timestamp_t timestamp = 0;
        auto aa = (static_cast<timestamp_t>(m_bytecode[  i]) << 56),
             ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
             ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
             da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
             ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
             fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
             ga = (static_cast<timestamp_t>(m_bytecode[++i]) <<  8),
             ha = (static_cast<timestamp_t>(m_bytecode[++i]));
        i++;
        timestamp = aa + ba + ca + da + ea + fa + ga + ha;
        os << "Timestamp: " << timestamp << "\n\n";

        std::vector<std::string> symbols;
        std::vector<std::string> values;
        std::vector<std::string> plugins;

        // reading the different tables, one after another

        if (b[i] == Instruction::SYM_TABLE_START)
        {
            os << "Symbols table:\n"; i++;
            uint16_t size = readNumber(i); i++;
            os << "Length: " << size << "\n";
            for (uint16_t j=0; j < size; ++j)
            {
                os << static_cast<int>(j) << ") ";
                std::string content = "";
                while (b[i] != 0)
                    content += b[i++];
                i++;
                os << content << "\n";
                symbols.push_back(content);
            }
            os << "\n";
        }
        else
        {
            os << termcolor::red << "Missing symbole table entry point\n" << termcolor::reset;
            return;
        }

        if (b[i] == Instruction::VAL_TABLE_START)
        {
            os << "Constants table:\n"; i++;
            uint16_t size = readNumber(i); i++;
            os << "Length: " << size << "\n";
            for (uint16_t j=0; j < size; ++j)
            {
                os << static_cast<int>(j) << ") ";
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
                    os << termcolor::red << "Unknown value type: " << static_cast<int>(type) << '\n' << termcolor::reset;
                    return;
                }
                os << "\n";
            }
            os << "\n";
        }
        else
        {
            os << termcolor::red << "Missing constant table entry point\n" << termcolor::reset;
            return;
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
                        os << "LOAD_SYMBOL " << termcolor::green << symbols[readNumber(i)] << "\n";
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
                        os << "STORE " << termcolor::green << symbols[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LET)
                    {
                        os << "LET " << termcolor::green << symbols[readNumber(i)] << "\n";
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
                    else if (inst == Instruction::CAPTURE)
                    {
                        os << "CAPTURE " << termcolor::reset << symbols[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::BUILTIN)
                    {
                        os << "BUILTIN " << termcolor::reset << Builtins::builtins[readNumber(i)].first << "\n";
                        i++;
                    }
                    else if (inst == Instruction::MUT)
                    {
                        os << "MUT " << termcolor::green << symbols[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::DEL)
                    {
                        os << "DEL " << termcolor::green << symbols[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::SAVE_ENV)
                        os << "SAVE_ENV\n";
                    else if (inst == Instruction::GET_FIELD)
                    {
                        os << "GET_FIELD " << termcolor::green << symbols[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::PLUGIN)
                    {
                        os << "PLUGIN " << termcolor::magenta << values[readNumber(i)] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LIST)
                    {
                        os << "LIST " << termcolor::reset << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::APPEND)
                    {
                        os << "APPEND " << termcolor::reset << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::CONCAT)
                    {
                        os << "CONCAT " << termcolor::reset << "(" << readNumber(i) << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::ADD)
                        os << "ADD\n";
                    else if (inst == Instruction::SUB)
                        os << "SUB\n";
                    else if (inst == Instruction::MUL)
                        os << "MUL\n";
                    else if (inst == Instruction::DIV)
                        os << "DIV\n";
                    else if (inst == Instruction::GT)
                        os << "GT\n";
                    else if (inst == Instruction::LT)
                        os << "LT\n";
                    else if (inst == Instruction::LE)
                        os << "LE\n";
                    else if (inst == Instruction::GE)
                        os << "GE\n";
                    else if (inst == Instruction::NEQ)
                        os << "NEQ\n";
                    else if (inst == Instruction::EQ)
                        os << "EQ\n";
                    else if (inst == Instruction::LEN)
                        os << "LEN\n";
                    else if (inst == Instruction::EMPTY)
                        os << "EMPTY\n";
                    else if (inst == Instruction::FIRSTOF)
                        os << "FIRSTOF\n";
                    else if (inst == Instruction::TAILOF)
                        os << "TAILOF\n";
                    else if (inst == Instruction::HEADOF)
                        os << "HEADOF\n";
                    else if (inst == Instruction::ISNIL)
                        os << "ISNIL\n";
                    else if (inst == Instruction::ASSERT)
                        os << "ASSERT\n";
                    else if (inst == Instruction::TO_NUM)
                        os << "TO_NUM\n";
                    else if (inst == Instruction::TO_STR)
                        os << "TO_STR\n";
                    else if (inst == Instruction::AT)
                        os << "AT\n";
                    else if (inst == Instruction::AND_)
                        os << "AND_\n";
                    else if (inst == Instruction::OR_)
                        os << "OR_\n";
                    else if (inst == Instruction::MOD)
                        os << "MOD\n";
                    else if (inst == Instruction::TYPE)
                        os << "TYPE\n";
                    else if (inst == Instruction::HASFIELD)
                        os << "HASFIELD\n";
                    else if (inst == Instruction::NOT)
                        os << "NOT\n";
                    else
                    {
                        os << termcolor::reset << "Unknown instruction: " << static_cast<int>(inst) << '\n' << termcolor::reset;
                        return;
                    }

                    if (i - j == size)
                        break;
                }
            }
            os << "\n" << termcolor::reset;
            ++pp;

            if (i == b.size())
                break;
        }
    }

    uint16_t BytecodeReader::readNumber(std::size_t& i)
    {
        uint16_t x = (static_cast<uint16_t>(m_bytecode[  i]) << 8),
                 y = static_cast<uint16_t>(m_bytecode[++i]);
        return x + y;
    }
}