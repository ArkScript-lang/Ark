#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <iomanip>
#include <termcolor/proxy.hpp>
#include <picosha2.h>

namespace Ark
{
    using namespace Ark::internal;

    void BytecodeReader::feed(const std::string& file)
    {
        std::ifstream ifs(file, std::ios::binary | std::ios::ate);
        if (!ifs.good())
            throw std::runtime_error("[BytecodeReader] Couldn't open file '" + file + "'");
        const std::size_t pos = ifs.tellg();
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

        // read major, minor and patch
        std::ignore = readNumber(i);
        i++;
        std::ignore = readNumber(i);
        i++;
        std::ignore = readNumber(i);
        i++;

        // reading the timestamp in big endian
        using timestamp_t = unsigned long long;
        timestamp_t timestamp = 0;
        const auto aa = (static_cast<timestamp_t>(m_bytecode[i]) << 56),
                   ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
                   ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
                   da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
                   ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
                   fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
                   ga = (static_cast<timestamp_t>(m_bytecode[++i]) << 8),
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

        if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == NOP))
        {
            os << "Invalid format";
            return;
        }

        uint16_t major = readNumber(i);
        i++;
        uint16_t minor = readNumber(i);
        i++;
        uint16_t patch = readNumber(i);
        i++;
        os << "Version:   " << major << "." << minor << "." << patch << "\n";

        using timestamp_t = unsigned long long;
        timestamp_t timestamp = 0;
        auto aa = (static_cast<timestamp_t>(m_bytecode[i]) << 56),
             ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
             ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
             da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
             ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
             fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
             ga = (static_cast<timestamp_t>(m_bytecode[++i]) << 8),
             ha = (static_cast<timestamp_t>(m_bytecode[++i]));
        i++;
        timestamp = aa + ba + ca + da + ea + fa + ga + ha;
        os << "Timestamp: " << timestamp << "\n";

        os << "SHA256:    ";
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
            os << std::hex << static_cast<int>(m_bytecode[i]);
            ++i;
        }
        os << "\n\n"
           << std::dec;

        std::vector<std::string> symbols;
        std::vector<std::string> values;

        // reading the different tables, one after another

        if ((sStart.has_value() && !sEnd.has_value()) || (!sStart.has_value() && sEnd.has_value()))
        {
            os << termcolor::red << "Both start and end parameter need to be provided together\n"
               << termcolor::reset;
            return;
        }
        if (sStart.has_value() && sEnd.has_value() && sStart.value() >= sEnd.value())
        {
            os << termcolor::red << "Invalid slice start and end arguments\n"
               << termcolor::reset;
            return;
        }

        if (b[i] == SYM_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            i++;
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
            os << termcolor::red << "Missing symbole table entry point\n"
               << termcolor::reset;
            return;
        }

        if (segment == BytecodeSegment::Symbols)
            return;

        if (b[i] == VAL_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            i++;
            uint16_t sliceSize = size;

            bool showVal = (segment == BytecodeSegment::All || segment == BytecodeSegment::Values);
            if (showVal && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                os << termcolor::red << "Slice start or end can't be greater than the segment size: " << size << "\n";
            else if (showVal && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showVal || segment == BytecodeSegment::HeadersOnly)
                os << termcolor::green << "Constants table" << termcolor::reset << " (length: " << sliceSize << ")\n";

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
            os << termcolor::red << "Missing constant table entry point\n"
               << termcolor::reset;
            return;
        }

        if (segment == BytecodeSegment::Values)
            return;

        uint16_t pp = 0;
        std::size_t cumulated_segment_size = i + 3;

        while (b[i] == Instruction::CODE_SEGMENT_START && (segment == BytecodeSegment::All || segment == BytecodeSegment::Code || segment == BytecodeSegment::HeadersOnly))
        {
            i++;
            uint16_t size = readNumber(i);
            i++;

            bool displayCode = true;

            if (auto page = cPage)
                displayCode = pp == page.value();

            if (displayCode)
                os << termcolor::magenta << "Code segment " << pp << termcolor::reset << " (length: " << size << ")\n";

            if (size == 0)
            {
                if (displayCode)
                    os << "NOP";
            }
            else
            {
                i += 4 * sStart.value_or(0);

                if (cPage.value_or(pp) == pp && segment != BytecodeSegment::HeadersOnly)
                {
                    if (sStart.has_value() && sEnd.has_value() && ((sStart.value() > size) || (sEnd.value() > size)))
                    {
                        os << termcolor::red << "Slice start or end can't be greater than the segment size: " << size << termcolor::reset << "\n";
                        return;
                    }

                    for (std::size_t j = sStart.value_or(0), end = sEnd.value_or(size); j < end; ++j)
                    {
                        [[maybe_unused]] uint8_t padding = b[i];
                        ++i;
                        uint8_t inst = b[i];
                        ++i;
                        uint16_t arg = readNumber(i);
                        ++i;

                        // instruction number
                        os << termcolor::cyan << j << " ";
                        // padding inst arg arg
                        os << termcolor::reset << std::hex
                           << std::setw(2) << std::setfill('0') << static_cast<int>(padding) << " "
                           << std::setw(2) << std::setfill('0') << static_cast<int>(inst) << " "
                           << std::setw(2) << std::setfill('0') << static_cast<int>(b[i - 2]) << " "
                           << std::setw(2) << std::setfill('0') << static_cast<int>(b[i - 1]) << " ";
                        // reset stream
                        os << std::dec << termcolor::yellow;

                        if (inst == NOP)
                            os << "NOP\n";
                        else if (inst == LOAD_SYMBOL)
                            os << "LOAD_SYMBOL " << termcolor::green << symbols[arg] << "\n";
                        else if (inst == LOAD_CONST)
                            os << "LOAD_CONST " << termcolor::magenta << values[arg] << "\n";
                        else if (inst == POP_JUMP_IF_TRUE)
                            os << "POP_JUMP_IF_TRUE " << termcolor::red << "(" << arg << ")\n";
                        else if (inst == STORE)
                            os << "STORE " << termcolor::green << symbols[arg] << "\n";
                        else if (inst == LET)
                            os << "LET " << termcolor::green << symbols[arg] << "\n";
                        else if (inst == POP_JUMP_IF_FALSE)
                            os << "POP_JUMP_IF_FALSE " << termcolor::red << "(" << arg << ")\n";
                        else if (inst == JUMP)
                            os << "JUMP " << termcolor::red << "(" << arg << ")\n";
                        else if (inst == RET)
                            os << "RET\n";
                        else if (inst == HALT)
                            os << "HALT\n";
                        else if (inst == CALL)
                            os << "CALL " << termcolor::reset << "(" << arg << ")\n";
                        else if (inst == CAPTURE)
                            os << "CAPTURE " << termcolor::reset << symbols[arg] << "\n";
                        else if (inst == BUILTIN)
                            os << "BUILTIN " << termcolor::reset << Builtins::builtins[arg].first << "\n";
                        else if (inst == MUT)
                            os << "MUT " << termcolor::green << symbols[arg] << "\n";
                        else if (inst == DEL)
                            os << "DEL " << termcolor::green << symbols[arg] << "\n";
                        else if (inst == SAVE_ENV)
                            os << "SAVE_ENV\n";
                        else if (inst == GET_FIELD)
                            os << "GET_FIELD " << termcolor::green << symbols[arg] << "\n";
                        else if (inst == PLUGIN)
                            os << "PLUGIN " << termcolor::magenta << values[arg] << "\n";
                        else if (inst == LIST)
                            os << "LIST " << termcolor::reset << "(" << arg << ")\n";
                        else if (inst == APPEND)
                            os << "APPEND " << termcolor::reset << "(" << arg << ")\n";
                        else if (inst == CONCAT)
                            os << "CONCAT " << termcolor::reset << "(" << arg << ")\n";
                        else if (inst == APPEND_IN_PLACE)
                            os << "APPEND_IN_PLACE " << termcolor::reset << "(" << arg << ")\n";
                        else if (inst == CONCAT_IN_PLACE)
                            os << "CONCAT_IN_PLACE " << termcolor::reset << "(" << arg << ")\n";
                        else if (inst == POP_LIST)
                            os << "POP_LIST " << termcolor::reset << "\n";
                        else if (inst == POP_LIST_IN_PLACE)
                            os << "POP_LIST_IN_PLACE " << termcolor::reset << "\n";
                        else if (inst == POP)
                            os << "POP\n";
                        else if (inst == ADD)
                            os << "ADD\n";
                        else if (inst == SUB)
                            os << "SUB\n";
                        else if (inst == MUL)
                            os << "MUL\n";
                        else if (inst == DIV)
                            os << "DIV\n";
                        else if (inst == GT)
                            os << "GT\n";
                        else if (inst == LT)
                            os << "LT\n";
                        else if (inst == LE)
                            os << "LE\n";
                        else if (inst == GE)
                            os << "GE\n";
                        else if (inst == NEQ)
                            os << "NEQ\n";
                        else if (inst == EQ)
                            os << "EQ\n";
                        else if (inst == LEN)
                            os << "LEN\n";
                        else if (inst == EMPTY)
                            os << "EMPTY\n";
                        else if (inst == TAIL)
                            os << "TAIL\n";
                        else if (inst == HEAD)
                            os << "HEAD\n";
                        else if (inst == ISNIL)
                            os << "ISNIL\n";
                        else if (inst == ASSERT)
                            os << "ASSERT\n";
                        else if (inst == TO_NUM)
                            os << "TO_NUM\n";
                        else if (inst == TO_STR)
                            os << "TO_STR\n";
                        else if (inst == AT)
                            os << "AT\n";
                        else if (inst == AND_)
                            os << "AND_\n";
                        else if (inst == OR_)
                            os << "OR_\n";
                        else if (inst == MOD)
                            os << "MOD\n";
                        else if (inst == TYPE)
                            os << "TYPE\n";
                        else if (inst == HASFIELD)
                            os << "HASFIELD\n";
                        else if (inst == NOT)
                            os << "NOT\n";
                        else
                        {
                            os << termcolor::reset << "Unknown instruction: " << static_cast<int>(inst) << '\n'
                               << termcolor::reset;
                            return;
                        }
                    }
                }

                i = cumulated_segment_size + size * 4;
                cumulated_segment_size += size * 4 + 3;
            }
            if (displayCode && segment != BytecodeSegment::HeadersOnly)
                os << "\n"
                   << termcolor::reset;

            ++pp;

            if (i == b.size())
                break;
        }
    }

    uint16_t BytecodeReader::readNumber(std::size_t& i) const
    {
        const uint16_t x = static_cast<uint16_t>(m_bytecode[i]) << 8;
        const uint16_t y = m_bytecode[++i];
        return x + y;
    }
}
