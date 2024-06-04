#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <iomanip>
#include <termcolor/proxy.hpp>
#include <picosha2.h>
#include <fmt/core.h>

namespace Ark
{
    using namespace Ark::internal;

    void BytecodeReader::feed(const bytecode_t& bytecode)
    {
        m_bytecode = bytecode;
    }

    void BytecodeReader::feed(const std::string& file)
    {
        std::ifstream ifs(file, std::ios::binary | std::ios::ate);
        if (!ifs.good())
            throw std::runtime_error(fmt::format("[BytecodeReader] Couldn't open file '{}'", file));

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

    bool BytecodeReader::checkMagic() const
    {
        return m_bytecode.size() >= 4 && m_bytecode[0] == 'a' &&
            m_bytecode[1] == 'r' && m_bytecode[2] == 'k' &&
            m_bytecode[3] == internal::Instruction::NOP;
    }


    const bytecode_t& BytecodeReader::bytecode() noexcept
    {
        return m_bytecode;
    }

    Version BytecodeReader::version() const
    {
        if (!checkMagic() || m_bytecode.size() < 10)
            return Version { 0, 0, 0 };

        return Version {
            .major = static_cast<uint16_t>((m_bytecode[4] << 8) + m_bytecode[5]),
            .minor = static_cast<uint16_t>((m_bytecode[6] << 8) + m_bytecode[7]),
            .patch = static_cast<uint16_t>((m_bytecode[8] << 8) + m_bytecode[9])
        };
    }


    unsigned long long BytecodeReader::timestamp() const
    {
        // 4 (ark\0) + version (2 bytes / number) + timestamp = 18 bytes
        if (!checkMagic() || m_bytecode.size() < 18)
            return 0;

        // reading the timestamp in big endian
        using timestamp_t = unsigned long long;
        return (static_cast<timestamp_t>(m_bytecode[10]) << 56) +
            (static_cast<timestamp_t>(m_bytecode[11]) << 48) +
            (static_cast<timestamp_t>(m_bytecode[12]) << 40) +
            (static_cast<timestamp_t>(m_bytecode[13]) << 32) +
            (static_cast<timestamp_t>(m_bytecode[14]) << 24) +
            (static_cast<timestamp_t>(m_bytecode[15]) << 16) +
            (static_cast<timestamp_t>(m_bytecode[16]) << 8) +
            static_cast<timestamp_t>(m_bytecode[17]);
    }

    std::vector<unsigned char> BytecodeReader::sha256() const
    {
        if (!checkMagic() || m_bytecode.size() < 18 + picosha2::k_digest_size)
            return {};

        std::vector<unsigned char> sha(picosha2::k_digest_size);
        for (std::size_t i = 0; i < picosha2::k_digest_size; ++i)
            sha[i] = m_bytecode[18 + i];
        return sha;
    }

    Symbols BytecodeReader::symbols() const
    {
        if (!checkMagic() || m_bytecode.size() < 18 + picosha2::k_digest_size ||
            m_bytecode[18 + picosha2::k_digest_size] != SYM_TABLE_START)
            return {};

        std::size_t i = 18 + picosha2::k_digest_size + 1;
        const uint16_t size = readNumber(i);
        i++;

        Symbols block;
        block.start = 18 + picosha2::k_digest_size;
        block.symbols.reserve(size);

        for (uint16_t j = 0; j < size; ++j)
        {
            std::string content;
            while (m_bytecode[i] != 0)
                content += m_bytecode[i++];
            i++;

            block.symbols.push_back(content);
        }

        block.end = i;
        return block;
    }

    Values BytecodeReader::values() const
    {
        if (!checkMagic())
            return {};

        const auto data = symbols();
        std::size_t i = data.end;
        if (m_bytecode[i] != VAL_TABLE_START)
            return {};
        i++;


        const uint16_t size = readNumber(i);
        i++;
        Values block;
        block.start = data.end;
        block.values.reserve(size);

        for (uint16_t j = 0; j < size; ++j)
        {
            const uint8_t type = m_bytecode[i];
            i++;

            if (type == NUMBER_TYPE)
            {
                std::string val;
                while (m_bytecode[i] != 0)
                    val.push_back(m_bytecode[i++]);
                block.values.emplace_back(std::stod(val));
            }
            else if (type == STRING_TYPE)
            {
                std::string val;
                while (m_bytecode[i] != 0)
                    val.push_back(m_bytecode[i++]);
                block.values.emplace_back(val);
            }
            else if (type == FUNC_TYPE)
            {
                const uint16_t addr = readNumber(i);
                i++;
                block.values.emplace_back(addr);
            }
            else
                throw std::runtime_error(fmt::format("Unknown value type: {:x}", type));
            i++;
        }

        block.end = i;
        return block;
    }

    Code BytecodeReader::code() const
    {
        if (!checkMagic())
            return {};

        const auto data = values();
        std::size_t i = data.end;

        Code block;
        block.start = i;

        while (m_bytecode[i] == CODE_SEGMENT_START)
        {
            i++;
            const std::size_t size = readNumber(i) * 4;
            i++;

            block.pages.emplace_back().reserve(size);
            for (std::size_t j = 0; j < size; ++j)
                block.pages.back().push_back(m_bytecode[i++]);

            if (i == m_bytecode.size())
                break;
        }

        return block;
    }


    void BytecodeReader::display(const BytecodeSegment segment,
                                 const std::optional<uint16_t> sStart,
                                 const std::optional<uint16_t> sEnd,
                                 const std::optional<uint16_t> cPage)
    {
        std::ostream& os = std::cout;

        if (!checkMagic())
        {
            os << "Invalid format";
            return;
        }

        auto [major, minor, patch] = version();
        os << "Version:   " << major << "." << minor << "." << patch << "\n";
        os << "Timestamp: " << timestamp() << "\n";
        os << "SHA256:    ";
        for (const auto sha = sha256(); unsigned char h : sha)
            os << fmt::format("{:02x}", h);
        os << "\n\n";

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

        const auto syms = symbols();
        const auto vals = values();
        const auto code_block = code();

        // symbols table
        {
            std::size_t size = syms.symbols.size();
            std::size_t sliceSize = size;
            bool showSym = (segment == BytecodeSegment::All || segment == BytecodeSegment::Symbols);

            if (showSym && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                os << termcolor::red << "Slice start or end can't be greater than the segment size: " << size << "\n";
            else if (showSym && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showSym || segment == BytecodeSegment::HeadersOnly)
                os << termcolor::cyan << "Symbols table" << termcolor::reset << " (length: " << sliceSize << ")\n";

            for (std::size_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showSym = showSym && (j >= start.value() && j <= end.value());

                if (showSym)
                    os << fmt::format("{}) {}\n", j, syms.symbols[j]);
            }

            if (showSym)
                os << "\n";
            if (segment == BytecodeSegment::Symbols)
                return;
        }

        // values table
        {
            std::size_t size = vals.values.size();
            std::size_t sliceSize = size;

            bool showVal = (segment == BytecodeSegment::All || segment == BytecodeSegment::Values);
            if (showVal && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                os << termcolor::red << "Slice start or end can't be greater than the segment size: " << size << "\n";
            else if (showVal && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showVal || segment == BytecodeSegment::HeadersOnly)
                os << termcolor::green << "Constants table" << termcolor::reset << " (length: " << sliceSize << ")\n";

            for (std::size_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showVal = showVal && (j >= start.value() && j <= end.value());

                if (showVal)
                {
                    switch (const auto val = vals.values[j]; val.valueType())
                    {
                        case ValueType::Number:
                            os << fmt::format("{}) (Number) {}\n", j, val.number());
                            break;
                        case ValueType::String:
                            os << fmt::format("{}) (String) {}\n", j, val.string());
                            break;
                        case ValueType::PageAddr:
                            os << fmt::format("{}) (PageAddr) {}\n", j, val.pageAddr());
                            break;
                        default:
                            os << termcolor::red << "Value type not handled: " << types_to_str[static_cast<std::size_t>(val.valueType())]
                               << '\n'
                               << termcolor::reset;
                            break;
                    }
                }
            }

            if (showVal)
                os << "\n";
            if (segment == BytecodeSegment::Values)
                return;
        }

        const auto stringify_value = [](const Value& val) -> std::string {
            switch (val.valueType())
            {
                case ValueType::Number:
                    return fmt::format("{} (Number)", val.number());
                case ValueType::String:
                    return fmt::format("{} (String)", val.string());
                case ValueType::PageAddr:
                    return fmt::format("{} (PageAddr)", val.pageAddr());
                default:
                    return "";
            }
        };

        if (segment == BytecodeSegment::All || segment == BytecodeSegment::Code || segment == BytecodeSegment::HeadersOnly)
        {
            uint16_t pp = 0;

            for (const auto& page : code_block.pages)
            {
                bool displayCode = true;

                if (auto wanted_page = cPage)
                    displayCode = pp == wanted_page.value();

                if (displayCode)
                    os << termcolor::magenta << "Code segment " << pp << termcolor::reset << " (length: " << page.size() << ")\n";

                if (page.empty())
                {
                    if (displayCode)
                        os << "NOP";
                }
                else
                {
                    if (cPage.value_or(pp) == pp && segment != BytecodeSegment::HeadersOnly)
                    {
                        if (sStart.has_value() && sEnd.has_value() && ((sStart.value() > page.size()) || (sEnd.value() > page.size())))
                        {
                            os << termcolor::red << "Slice start or end can't be greater than the segment size: " << page.size() << termcolor::reset << "\n";
                            return;
                        }

                        for (std::size_t j = sStart.value_or(0), end = sEnd.value_or(page.size()); j < end; j += 4)
                        {
                            const uint8_t padding = page[j];
                            const uint8_t inst = page[j + 1];
                            const uint16_t arg = static_cast<uint16_t>((page[j + 2] << 8) + page[j + 3]);

                            // instruction number
                            os << termcolor::cyan << fmt::format("{:>4}", j / 4) << termcolor::reset;
                            // padding inst arg arg
                            os << fmt::format(" {:02x} {:02x} {:02x} {:02x} ", padding, inst, page[j + 2], page[j + 3]);
                            os << termcolor::yellow;

                            if (inst == NOP)
                                os << "NOP\n";
                            else if (inst == LOAD_SYMBOL)
                                os << "LOAD_SYMBOL " << termcolor::green << syms.symbols[arg] << "\n";
                            else if (inst == LOAD_CONST)
                                os << "LOAD_CONST " << termcolor::magenta << stringify_value(vals.values[arg]) << "\n";
                            else if (inst == POP_JUMP_IF_TRUE)
                                os << "POP_JUMP_IF_TRUE " << termcolor::red << "(" << arg << ")\n";
                            else if (inst == STORE)
                                os << "STORE " << termcolor::green << syms.symbols[arg] << "\n";
                            else if (inst == LET)
                                os << "LET " << termcolor::green << syms.symbols[arg] << "\n";
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
                                os << "CAPTURE " << termcolor::reset << syms.symbols[arg] << "\n";
                            else if (inst == BUILTIN)
                                os << "BUILTIN " << termcolor::reset << Builtins::builtins[arg].first << "\n";
                            else if (inst == MUT)
                                os << "MUT " << termcolor::green << syms.symbols[arg] << "\n";
                            else if (inst == DEL)
                                os << "DEL " << termcolor::green << syms.symbols[arg] << "\n";
                            else if (inst == SAVE_ENV)
                                os << "SAVE_ENV\n";
                            else if (inst == GET_FIELD)
                                os << "GET_FIELD " << termcolor::green << syms.symbols[arg] << "\n";
                            else if (inst == PLUGIN)
                                os << "PLUGIN " << termcolor::magenta << stringify_value(vals.values[arg]) << "\n";
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
                                os << termcolor::reset << fmt::format("Unknown instruction: {:02x}", inst) << '\n'
                                   << termcolor::reset;
                                return;
                            }
                        }
                    }
                }
                if (displayCode && segment != BytecodeSegment::HeadersOnly)
                    os << "\n"
                       << termcolor::reset;

                ++pp;
            }
        }
    }

    uint16_t BytecodeReader::readNumber(std::size_t& i) const
    {
        const uint16_t x = static_cast<uint16_t>(m_bytecode[i]) << 8;
        const uint16_t y = m_bytecode[++i];
        return x + y;
    }
}
