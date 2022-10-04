#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>
#include <Ark/Utils.hpp>

#include <iomanip>
#include <picosha2.h>
#include <fmt/color.h>

namespace Ark
{
    using namespace Ark::internal;

    void BytecodeReader::feed(const std::string& file)
    {
        std::ifstream ifs(file, std::ios::binary | std::ios::ate);
        if (!ifs.good())
            throw std::runtime_error("BytecodeReader - Couldn't open file '" + file + "'");
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

        i += 6;  // skip the version

        // reading the timestamp in big endian
        using timestamp_t = unsigned long long;
        auto aa = (static_cast<timestamp_t>(m_bytecode[i]) << 56),
             ba = (static_cast<timestamp_t>(m_bytecode[++i]) << 48),
             ca = (static_cast<timestamp_t>(m_bytecode[++i]) << 40),
             da = (static_cast<timestamp_t>(m_bytecode[++i]) << 32),
             ea = (static_cast<timestamp_t>(m_bytecode[++i]) << 24),
             fa = (static_cast<timestamp_t>(m_bytecode[++i]) << 16),
             ga = (static_cast<timestamp_t>(m_bytecode[++i]) << 8),
             ha = (static_cast<timestamp_t>(m_bytecode[++i]));

        return (aa + ba + ca + da + ea + fa + ga + ha);
    }

    void BytecodeReader::display(const BytecodeSegment segment,
                                 const std::optional<uint16_t> sStart,
                                 const std::optional<uint16_t> sEnd,
                                 const std::optional<uint16_t> cPage)
    {
        bytecode_t b = bytecode();
        std::size_t i = 0;

        if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
        {
            fmt::print("Invalid format\n");
            return;
        }

        uint16_t major = readNumber(i);
        i++;
        uint16_t minor = readNumber(i);
        i++;
        uint16_t patch = readNumber(i);
        i++;

        fmt::print("Version:   {}.{}.{}\n", major, minor, patch);
        fmt::print("Timestamp: {}\n", timestamp());
        i += 8;  // jump over the timestamp

        fmt::print("SHA256:    ");
        for (std::size_t j = 0; j < picosha2::k_digest_size; ++j)
        {
            fmt::print("{:x}", m_bytecode[i]);
            ++i;
        }
        fmt::print("\n\n");

        std::vector<std::string> symbols;
        std::vector<std::string> values;

        // reading the different tables, one after another

        if ((sStart.has_value() && !sEnd.has_value()) || (!sStart.has_value() && sEnd.has_value()))
        {
            fmt::print(fmt::fg(fmt::color::red), "Both start and end parameter need to be provided together\n");
            return;
        }
        else if (sStart.has_value() && sEnd.has_value() && sStart.value() >= sEnd.value())
        {
            fmt::print(fmt::fg(fmt::color::red), "Invalid slice start and end arguments\n");
            return;
        }

        if (b[i] == Instruction::SYM_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            i++;
            uint16_t sliceSize = size;
            bool showSym = (segment == BytecodeSegment::All || segment == BytecodeSegment::Symbols);

            if (showSym && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", size);
            else if (showSym && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showSym || segment == BytecodeSegment::HeadersOnly)
                fmt::print("{} (length: {})\n", fmt::styled("Symbols table", fmt::fg(fmt::color::cyan)), sliceSize);

            for (uint16_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showSym = showSym && (j >= start.value() && j <= end.value());

                std::string content;
                while (b[i] != 0)
                    content += b[i++];
                i++;

                if (showSym)
                    fmt::print("{}) {}\n", j, content);

                symbols.push_back(content);
            }
            if (showSym)
                fmt::print("\n");
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "Missing symbole table entry point\n");
            return;
        }

        if (segment == BytecodeSegment::Symbols)
            return;

        if (b[i] == Instruction::VAL_TABLE_START)
        {
            i++;
            uint16_t size = readNumber(i);
            i++;
            uint16_t sliceSize = size;

            bool showVal = (segment == BytecodeSegment::All || segment == BytecodeSegment::Values);
            if (showVal && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", size);
            else if (showVal && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showVal || segment == BytecodeSegment::HeadersOnly)
                fmt::print("{} (length: {})\n", fmt::styled("Constants table", fmt::fg(fmt::color::green)), sliceSize);

            for (uint16_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showVal = showVal && (j >= start.value() && j <= end.value());

                if (showVal)
                    fmt::print("{}) ", j);
                uint8_t type = b[i];
                i++;

                if (type == Instruction::NUMBER_TYPE)
                {
                    std::string val;
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;
                    if (showVal)
                        fmt::print("(Number) {}", val);
                    values.push_back("(Number) " + val);
                }
                else if (type == Instruction::STRING_TYPE)
                {
                    std::string val;
                    while (b[i] != 0)
                        val.push_back(b[i++]);
                    i++;
                    if (showVal)
                        fmt::print("(String) {}", val);
                    values.push_back("(String) " + val);
                }
                else if (type == Instruction::FUNC_TYPE)
                {
                    uint16_t addr = readNumber(i);
                    i++;
                    if (showVal)
                        fmt::print("(PageAddr) {}", addr);
                    values.push_back("(PageAddr) " + std::to_string(addr));
                    i++;
                }
                else
                {
                    fmt::print(fmt::fg(fmt::color::red), "Unknown value type: {}\n", type);
                    return;
                }

                if (showVal)
                    fmt::print("\n");
            }

            if (showVal)
                fmt::print("\n");
        }
        else
        {
            fmt::print(fmt::fg(fmt::color::red), "Missing constant table entry point\n");
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
                fmt::print("{} {} (length: {})\n", fmt::styled("Code segment", fmt::fg(fmt::color::magenta)), pp, size);

            if (size == 0 && displayCode)
                fmt::print("NOP");
            else
            {
                i += 4 * sStart.value_or(0);

                if (cPage.value_or(pp) == pp && segment != BytecodeSegment::HeadersOnly)
                {
                    if (sStart.has_value() && sEnd.has_value() && ((sStart.value() > size) || (sEnd.value() > size)))
                    {
                        fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", size);
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

                        // instruction number followed by padding, inst, arg arg
                        fmt::print(fmt::fg(fmt::color::cyan), "{:>3} {:0>2x} {:0>2x} {:0>2x}{:0>2x} ", j, padding, inst, b[i - 2], b[i - 1]);

                        auto print_inst = [](const std::string& name) {
                            fmt::print(fmt::fg(fmt::color::yellow), "{}\n", name);
                        };
                        auto print_inst_with_arg = [](const std::string& name, auto arg, fmt::color color = fmt::color::white) {
                            fmt::print("{} ({})\n", fmt::styled(name, fmt::fg(fmt::color::yellow)), fmt::styled(arg, fmt::fg(color)));
                        };

                        if (inst == Instruction::NOP)
                            print_inst("NOP");
                        else if (inst == Instruction::LOAD_SYMBOL)
                            print_inst_with_arg("LOAD_SYMBOL", symbols[arg], fmt::color::green);
                        else if (inst == Instruction::LOAD_CONST)
                            print_inst_with_arg("LOAD_CONST", values[arg], fmt::color::magenta);
                        else if (inst == Instruction::POP_JUMP_IF_TRUE)
                            print_inst_with_arg("POP_JUMP_IF_TRUE", arg, fmt::color::red);
                        else if (inst == Instruction::STORE)
                            print_inst_with_arg("STORE", symbols[arg], fmt::color::green);
                        else if (inst == Instruction::LET)
                            print_inst_with_arg("LET", symbols[arg], fmt::color::green);
                        else if (inst == Instruction::POP_JUMP_IF_FALSE)
                            print_inst_with_arg("POP_JUMP_IF_FALSE", arg, fmt::color::red);
                        else if (inst == Instruction::JUMP)
                            print_inst_with_arg("JUMP", arg, fmt::color::red);
                        else if (inst == Instruction::RET)
                            print_inst("RET");
                        else if (inst == Instruction::HALT)
                            print_inst("HALT");
                        else if (inst == Instruction::CALL)
                            print_inst_with_arg("CALL", arg);
                        else if (inst == Instruction::CAPTURE)
                            print_inst_with_arg("CAPTURE", symbols[arg]);
                        else if (inst == Instruction::BUILTIN)
                            print_inst_with_arg("BUILTIN", Builtins::builtins[arg].first);
                        else if (inst == Instruction::MUT)
                            print_inst_with_arg("MUT", symbols[arg], fmt::color::green);
                        else if (inst == Instruction::DEL)
                            print_inst_with_arg("DEL", symbols[arg], fmt::color::green);
                        else if (inst == Instruction::SAVE_ENV)
                            print_inst("SAVE_ENV");
                        else if (inst == Instruction::GET_FIELD)
                            print_inst_with_arg("GET_FIELD", symbols[arg], fmt::color::green);
                        else if (inst == Instruction::PLUGIN)
                            print_inst_with_arg("PLUGIN", values[arg], fmt::color::magenta);
                        else if (inst == Instruction::LIST)
                            print_inst_with_arg("LIST", arg);
                        else if (inst == Instruction::APPEND)
                            print_inst_with_arg("APPEND", arg);
                        else if (inst == Instruction::CONCAT)
                            print_inst_with_arg("CONCAT", arg);
                        else if (inst == Instruction::APPEND_IN_PLACE)
                            print_inst_with_arg("APPEND_IN_PLACE", arg);
                        else if (inst == Instruction::CONCAT_IN_PLACE)
                            print_inst_with_arg("CONCAT_IN_PLACE", arg);
                        else if (inst == Instruction::POP_LIST)
                            print_inst("POP_LIST");
                        else if (inst == Instruction::POP_LIST_IN_PLACE)
                            print_inst("POP_LIST_IN_PLACE");
                        else if (inst == Instruction::POP)
                            print_inst("POP");
                        else if (inst == Instruction::ADD)
                            print_inst("ADD");
                        else if (inst == Instruction::SUB)
                            print_inst("SUB");
                        else if (inst == Instruction::MUL)
                            print_inst("MUL");
                        else if (inst == Instruction::DIV)
                            print_inst("DIV");
                        else if (inst == Instruction::GT)
                            print_inst("GT");
                        else if (inst == Instruction::LT)
                            print_inst("LT");
                        else if (inst == Instruction::LE)
                            print_inst("LE");
                        else if (inst == Instruction::GE)
                            print_inst("GE");
                        else if (inst == Instruction::NEQ)
                            print_inst("NEQ");
                        else if (inst == Instruction::EQ)
                            print_inst("EQ");
                        else if (inst == Instruction::LEN)
                            print_inst("LEN");
                        else if (inst == Instruction::EMPTY)
                            print_inst("EMPTY");
                        else if (inst == Instruction::TAIL)
                            print_inst("TAIL");
                        else if (inst == Instruction::HEAD)
                            print_inst("HEAD");
                        else if (inst == Instruction::ISNIL)
                            print_inst("ISNIL");
                        else if (inst == Instruction::ASSERT)
                            print_inst("ASSERT");
                        else if (inst == Instruction::TO_NUM)
                            print_inst("TO_NUM");
                        else if (inst == Instruction::TO_STR)
                            print_inst("TO_STR");
                        else if (inst == Instruction::AT)
                            print_inst("AT");
                        else if (inst == Instruction::AND_)
                            print_inst("AND_");
                        else if (inst == Instruction::OR_)
                            print_inst("OR_");
                        else if (inst == Instruction::MOD)
                            print_inst("MOD");
                        else if (inst == Instruction::TYPE)
                            print_inst("TYPE");
                        else if (inst == Instruction::HASFIELD)
                            print_inst("HASFIELD");
                        else if (inst == Instruction::NOT)
                            print_inst("NOT");
                        else
                        {
                            fmt::print(fmt::fg(fmt::color::red), "Unknown instruction: {}\n", inst);
                            return;
                        }
                    }
                }

                i = cumulated_segment_size + size * 4;
                cumulated_segment_size += size * 4 + 3;
            }
            if (displayCode && segment != BytecodeSegment::HeadersOnly)
                fmt::print("\n");

            ++pp;

            if (i == b.size())
                break;
        }
    }

    uint16_t BytecodeReader::readNumber(std::size_t& i)
    {
        uint16_t x = (static_cast<uint16_t>(m_bytecode[i]) << 8),
                 y = static_cast<uint16_t>(m_bytecode[++i]);
        return x + y;
    }
}
