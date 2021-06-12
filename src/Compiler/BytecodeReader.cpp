#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>
#undef abs
#include <Ark/Utils.hpp>

#include <termcolor.hpp>
#include <picosha2.hpp>

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
        for (std::size_t i = 0; i < pos; ++i)
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

    void BytecodeReader::display(const BytecodeSegment segment,
                                 const std::optional<uint16_t> sStart,
                                 const std::optional<uint16_t> sEnd,
                                 const std::optional<uint16_t> cPage)
    {
        bytecode_t b = bytecode();
        std::size_t i = 0;

        std::ostream& os = std::cout;

        if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
        {
            os << "Invalid format";
            return;
        }

        uint16_t major = readNumber(i); i++;
        uint16_t minor = readNumber(i); i++;
        uint16_t patch = readNumber(i); i++;
        os << "Version:   " << major << "." << minor << "." << patch << "\n";

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
        os << "Timestamp: " << timestamp << "\n";

        os << "SHA256:    ";
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
            os << std::hex << static_cast<int>(m_bytecode[i]) << std::dec;
            ++i;
        }
        os << "\n\n";

        std::vector<std::string> symbols;
        std::vector<std::string> values;

        // reading the different tables, one after another

        if ((sStart.has_value() && !sEnd.has_value()) || (!sStart.has_value() && sEnd.has_value()))
        {
            os << termcolor::red << "Both start and end parameter need to be provided together\n" << termcolor::reset;
            return;
        }
        else if (sStart.has_value() && sEnd.has_value() && sStart.value() >= sEnd.value())
        {
            os << termcolor::red << "Invalid slice start and end arguments\n" << termcolor::reset;
            return;
        }

        if (b[i] == Instruction::SYM_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i); i++;
            uint16_t sliceSize = size;
            bool showSym = (segment == BytecodeSegment::All || segment == BytecodeSegment::Symbols);

            if (showSym && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                os << termcolor::red << "Slice start or end can't be greater than the segment size: " << size << "\n";
            else if (showSym && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showSym || segment == BytecodeSegment::HeadersOnly)
                os << termcolor::cyan << "Symbols table" << termcolor::reset << " (length: " << sliceSize << ")\n";

            for (uint16_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showSym = showSym && (j >= start.value() && j <= end.value());

                std::string content;
                while (b[i] != 0)
                    content += b[i++];
                i++;

                if (showSym)
                {
                    os << static_cast<int>(j) << ") ";
                    os << content << "\n";
                }

                symbols.push_back(content);
            }
            if (showSym)
                os << "\n";
        }
        else
        {
            os << termcolor::red << "Missing symbole table entry point\n" << termcolor::reset;
            return;
        }

        if (segment == BytecodeSegment::Symbols)
            return;

        if (b[i] == Instruction::VAL_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i); i++;
            uint16_t sliceSize = size;

            bool showVal = (segment == BytecodeSegment::All || segment == BytecodeSegment::Values);
            if (showVal && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                os << termcolor::red << "Slice start or end can't be greater than the segment size: " << size << "\n";
            else if (showVal && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showVal || segment == BytecodeSegment::HeadersOnly)
                os << termcolor::green << "Constants table" << termcolor::reset << " (length: " << sliceSize << ")\n";

            bool showLine = true;
            for (uint16_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showVal = showVal && (j >= start.value() && j <= end.value());

                if (showVal)
                    os << static_cast<int>(j) << ") ";
                uint8_t type = b[i];
                i++;

                if (type == Instruction::NUMBER_TYPE)
                {
                    std::string val;
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;
                    if (showVal)
                        os << "(Number) " << val;
                    values.push_back("(Number) " + val);
                }
                else if (type == Instruction::STRING_TYPE)
                {
                    std::string val;
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;
                    if (showVal)
                        os << "(String) " << val;
                    values.push_back("(String) " + val);
                }
                else if (type == Instruction::FUNC_TYPE)
                {
                    uint16_t addr = readNumber(i);
                    i++;
                    if (showVal)
                        os << "(PageAddr) " << addr;
                    values.push_back("(PageAddr) " + std::to_string(addr));
                    i++;
                }
                else
                {
                    os << termcolor::red << "Unknown value type: " << static_cast<int>(type) << '\n'
                       << termcolor::reset;
                    return;
                }

                if (showVal)
                    os << "\n";
            }

            if (showVal)
                os << "\n";
        }
        else
        {
            os << termcolor::red << "Missing constant table entry point\n" << termcolor::reset;
            return;
        }

        if (segment == BytecodeSegment::Values)
            return;

        uint16_t pp = 0;

        while (b[i] == Instruction::CODE_SEGMENT_START && (segment == BytecodeSegment::All || segment == BytecodeSegment::Code || segment == BytecodeSegment::HeadersOnly))
        {
            i++;
            uint16_t size = readNumber(i); i++;
            uint16_t sliceSize = size;

            if (sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            bool displayCode = true;

            if (auto page = cPage)
                displayCode = pp == page.value();

            if (displayCode)
                os << termcolor::magenta << "Code segment " << pp << termcolor::reset << " (length: " << sliceSize << ")\n";

            if (size == 0)
            {
                if (displayCode)
                    os << "NOP";
            }
            else
            {
                uint16_t j = i;

                bool displayLine = segment == BytecodeSegment::HeadersOnly ? false : displayCode;
                while (true)
                {
                    uint16_t line_number = i - j;
                    if (sStart.has_value() && sEnd.has_value() && ((sStart.value() > size) || (sEnd.value() > size)))
                    {
                        os << termcolor::red << "Slice start or end can't be greater than the segment size: "  << size << termcolor::reset << "\n";
                        return;
                    }
                    else if (sStart.has_value() && sEnd.has_value() && cPage.has_value())
                        displayLine = displayCode && (line_number >= sStart.value() && line_number <= sEnd.value());

                    if (displayLine)
                        os << termcolor::cyan << line_number << termcolor::reset << " " << termcolor::yellow;
                    uint8_t inst = b[i];
                    i++;

                    if (inst == Instruction::NOP)
                    {
                        if (displayLine)
                            os << "NOP\n";
                    }
                    else if (inst == Instruction::LOAD_SYMBOL)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "LOAD_SYMBOL " << termcolor::green << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LOAD_CONST)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "LOAD_CONST " << termcolor::magenta << values[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::POP_JUMP_IF_TRUE)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "POP_JUMP_IF_TRUE " << termcolor::red << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::STORE)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "STORE " << termcolor::green << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LET)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "LET " << termcolor::green << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::POP_JUMP_IF_FALSE)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "POP_JUMP_IF_FALSE " << termcolor::red << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::JUMP)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "JUMP " << termcolor::red << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::RET)
                    {
                        if (displayLine)
                            os << "RET\n";
                    }
                    else if (inst == Instruction::HALT)
                    {
                        if (displayLine)
                            os << "HALT\n";
                    }
                    else if (inst == Instruction::CALL)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "CALL " << termcolor::reset << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::CAPTURE)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "CAPTURE " << termcolor::reset << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::BUILTIN)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "BUILTIN " << termcolor::reset << Builtins::builtins[index].first << "\n";
                        i++;
                    }
                    else if (inst == Instruction::MUT)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "MUT " << termcolor::green << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::DEL)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "DEL " << termcolor::green << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::SAVE_ENV)
                    {
                        if (displayLine)
                            os << "SAVE_ENV\n";
                    }
                    else if (inst == Instruction::GET_FIELD)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "GET_FIELD " << termcolor::green << symbols[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::PLUGIN)
                    {
                        uint16_t index = readNumber(i);
                        if (displayLine)
                            os << "PLUGIN " << termcolor::magenta << values[index] << "\n";
                        i++;
                    }
                    else if (inst == Instruction::LIST)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "LIST " << termcolor::reset << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::APPEND)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "APPEND " << termcolor::reset << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::CONCAT)
                    {
                        uint16_t value = readNumber(i);
                        if (displayLine)
                            os << "CONCAT " << termcolor::reset << "(" << value << ")\n";
                        i++;
                    }
                    else if (inst == Instruction::ADD)
                    {
                        if (displayLine)
                            os << "ADD\n";
                    }
                    else if (inst == Instruction::SUB)
                    {
                        if (displayLine)
                            os << "SUB\n";
                    }
                    else if (inst == Instruction::MUL)
                    {
                        if (displayLine)
                            os << "MUL\n";
                    }
                    else if (inst == Instruction::DIV)
                    {
                        if (displayLine)
                            os << "DIV\n";
                    }
                    else if (inst == Instruction::GT)
                    {
                        if (displayLine)
                            os << "GT\n";
                    }
                    else if (inst == Instruction::LT)
                    {
                        if (displayLine)
                            os << "LT\n";
                    }
                    else if (inst == Instruction::LE)
                    {
                        if (displayLine)
                            os << "LE\n";
                    }
                    else if (inst == Instruction::GE)
                    {
                        if (displayLine)
                            os << "GE\n";
                    }
                    else if (inst == Instruction::NEQ)
                    {
                        if (displayLine)
                            os << "NEQ\n";
                    }
                    else if (inst == Instruction::EQ)
                    {
                        if (displayLine)
                            os << "EQ\n";
                    }
                    else if (inst == Instruction::LEN)
                    {
                        if (displayLine)
                            os << "LEN\n";
                    }
                    else if (inst == Instruction::EMPTY)
                    {
                        if (displayLine)
                            os << "EMPTY\n";
                    }
                    else if (inst == Instruction::TAIL)
                    {
                        if (displayLine)
                            os << "TAIL\n";
                    }
                    else if (inst == Instruction::HEAD)
                    {
                        if (displayLine)
                            os << "HEAD\n";
                    }
                    else if (inst == Instruction::ISNIL)
                    {
                        if (displayLine)
                            os << "ISNIL\n";
                    }
                    else if (inst == Instruction::ASSERT)
                    {
                        if (displayLine)
                            os << "ASSERT\n";
                    }
                    else if (inst == Instruction::TO_NUM)
                    {
                        if (displayLine)
                            os << "TO_NUM\n";
                    }
                    else if (inst == Instruction::TO_STR)
                    {
                        if (displayLine)
                            os << "TO_STR\n";
                    }
                    else if (inst == Instruction::AT)
                    {
                        if (displayLine)
                            os << "AT\n";
                    }
                    else if (inst == Instruction::AND_)
                    {
                        if (displayLine)
                            os << "AND_\n";
                    }
                    else if (inst == Instruction::OR_)
                    {
                        if (displayLine)
                            os << "OR_\n";
                    }
                    else if (inst == Instruction::MOD)
                    {
                        if (displayLine)
                            os << "MOD\n";
                    }
                    else if (inst == Instruction::TYPE)
                    {
                        if (displayLine)
                            os << "TYPE\n";
                    }
                    else if (inst == Instruction::HASFIELD)
                    {
                        if (displayLine)
                            os << "HASFIELD\n";
                    }
                    else if (inst == Instruction::NOT)
                    {
                        if (displayLine)
                            os << "NOT\n";
                    }
                    else
                    {
                        if (displayLine)
                            os << termcolor::reset << "Unknown instruction: " << static_cast<int>(inst) << '\n' << termcolor::reset;
                        return;
                    }

                    if (i - j == size)
                        break;
                }
            }
            if (displayCode && segment != BytecodeSegment::HeadersOnly)
                os << "\n" << termcolor::reset;

            if (cPage.has_value() && pp == cPage)
                return;

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