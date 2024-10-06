#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <iomanip>
#include <picosha2.h>
#include <fmt/core.h>
#include <fmt/color.h>

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

        const auto pos = ifs.tellg();
        // reserve appropriate number of bytes
        std::vector<char> temp(static_cast<std::size_t>(pos));
        ifs.seekg(0, std::ios::beg);
        ifs.read(&temp[0], pos);
        ifs.close();

        m_bytecode = bytecode_t(static_cast<std::size_t>(pos));
        for (std::size_t i = 0; i < static_cast<std::size_t>(pos); ++i)
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
                content.push_back(static_cast<char>(m_bytecode[i++]));
            i++;

            block.symbols.push_back(content);
        }

        block.end = i;
        return block;
    }

    Values BytecodeReader::values(const Symbols& symbols) const
    {
        if (!checkMagic())
            return {};

        std::size_t i = symbols.end;
        if (m_bytecode[i] != VAL_TABLE_START)
            return {};
        i++;

        const uint16_t size = readNumber(i);
        i++;
        Values block;
        block.start = symbols.end;
        block.values.reserve(size);

        for (uint16_t j = 0; j < size; ++j)
        {
            const uint8_t type = m_bytecode[i];
            i++;

            if (type == NUMBER_TYPE)
            {
                std::string val;
                while (m_bytecode[i] != 0)
                    val.push_back(static_cast<char>(m_bytecode[i++]));
                block.values.emplace_back(std::stod(val));
            }
            else if (type == STRING_TYPE)
            {
                std::string val;
                while (m_bytecode[i] != 0)
                    val.push_back(static_cast<char>(m_bytecode[i++]));
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

    Code BytecodeReader::code(const Values& values) const
    {
        if (!checkMagic())
            return {};

        std::size_t i = values.end;

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
                                 const std::optional<uint16_t> cPage) const
    {
        if (!checkMagic())
        {
            fmt::print("Invalid format");
            return;
        }

        auto [major, minor, patch] = version();
        fmt::println("Version:   {}.{}.{}", major, minor, patch);
        fmt::println("Timestamp: {}", timestamp());
        fmt::print("SHA256:    ");
        for (const auto sha = sha256(); unsigned char h : sha)
            fmt::print("{:02x}", h);
        fmt::print("\n\n");

        // reading the different tables, one after another

        if ((sStart.has_value() && !sEnd.has_value()) || (!sStart.has_value() && sEnd.has_value()))
        {
            fmt::print(fmt::fg(fmt::color::red), "Both start and end parameter need to be provided together\n");
            return;
        }
        if (sStart.has_value() && sEnd.has_value() && sStart.value() >= sEnd.value())
        {
            fmt::print(fmt::fg(fmt::color::red), "Invalid slice start and end arguments\n");
            return;
        }

        const auto syms = symbols();
        const auto vals = values(syms);
        const auto code_block = code(vals);

        // symbols table
        {
            std::size_t size = syms.symbols.size();
            std::size_t sliceSize = size;
            bool showSym = (segment == BytecodeSegment::All || segment == BytecodeSegment::Symbols);

            if (showSym && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", size);
            else if (showSym && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showSym || segment == BytecodeSegment::HeadersOnly)
                fmt::println("{} (length: {})", fmt::styled("Symbols table", fmt::fg(fmt::color::cyan)), sliceSize);

            for (std::size_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showSym = showSym && (j >= start.value() && j <= end.value());

                if (showSym)
                    fmt::println("{}) {}", j, syms.symbols[j]);
            }

            if (showSym)
                fmt::print("\n");
            if (segment == BytecodeSegment::Symbols)
                return;
        }

        // values table
        {
            std::size_t size = vals.values.size();
            std::size_t sliceSize = size;

            bool showVal = (segment == BytecodeSegment::All || segment == BytecodeSegment::Values);
            if (showVal && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", size);
            else if (showVal && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showVal || segment == BytecodeSegment::HeadersOnly)
                fmt::println("{} (length: {})", fmt::styled("Constants table", fmt::fg(fmt::color::cyan)), sliceSize);

            for (std::size_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showVal = showVal && (j >= start.value() && j <= end.value());

                if (showVal)
                {
                    switch (const auto val = vals.values[j]; val.valueType())
                    {
                        case ValueType::Number:
                            fmt::println("{}) (Number) {}", j, val.number());
                            break;
                        case ValueType::String:
                            fmt::println("{}) (String) {}", j, val.string());
                            break;
                        case ValueType::PageAddr:
                            fmt::println("{}) (PageAddr) {}", j, val.pageAddr());
                            break;
                        default:
                            fmt::print(fmt::fg(fmt::color::red), "Value type not handled: {}\n", types_to_str[static_cast<std::size_t>(val.valueType())]);
                            break;
                    }
                }
            }

            if (showVal)
                fmt::print("\n");
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

        enum class ArgKind
        {
            Symbol,
            Value,
            Builtin,
            Raw
        };

        struct Arg
        {
            ArgKind kind;
            uint16_t arg;
        };

        const auto color_print_inst = [&syms, &vals, &stringify_value](const std::string& name, std::optional<Arg> arg = std::nullopt) {
            fmt::print("{}", fmt::styled(name, fmt::fg(fmt::color::gold)));
            if (arg.has_value())
            {
                switch (auto [kind, idx] = arg.value(); kind)
                {
                    case ArgKind::Symbol:
                        fmt::print(fmt::fg(fmt::color::green), " {}\n", syms.symbols[idx]);
                        break;
                    case ArgKind::Value:
                        fmt::print(fmt::fg(fmt::color::magenta), " {}\n", stringify_value(vals.values[idx]));
                        break;
                    case ArgKind::Builtin:
                        fmt::print(" {}\n", Builtins::builtins[idx].first);
                        break;
                    case ArgKind::Raw:
                        fmt::print(fmt::fg(fmt::color::red), " ({})\n", idx);
                        break;
                }
            }
            else
                fmt::print("\n");
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
                    fmt::println(
                        "{} {} (length: {})",
                        fmt::styled("Code segment", fmt::fg(fmt::color::magenta)),
                        fmt::styled(pp, fmt::fg(fmt::color::magenta)),
                        page.size());

                if (page.empty())
                {
                    if (displayCode)
                        fmt::print("NOP");
                }
                else
                {
                    if (cPage.value_or(pp) != pp)
                        continue;
                    if (segment == BytecodeSegment::HeadersOnly)
                        continue;
                    if (sStart.has_value() && sEnd.has_value() && ((sStart.value() > page.size()) || (sEnd.value() > page.size())))
                    {
                        fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", page.size());
                        return;
                    }

                    for (std::size_t j = sStart.value_or(0), end = sEnd.value_or(page.size()); j < end; j += 4)
                    {
                        const uint8_t inst = page[j];
                        // TEMP
                        const uint8_t padding = page[j + 1];
                        const auto arg = static_cast<uint16_t>((page[j + 2] << 8) + page[j + 3]);

                        // instruction number
                        fmt::print(fmt::fg(fmt::color::cyan), "{:>4}", j / 4);
                        // padding inst arg arg
                        fmt::print(" {:02x} {:02x} {:02x} {:02x} ", inst, padding, page[j + 2], page[j + 3]);

                        if (inst == NOP)
                            color_print_inst("NOP");
                        else if (inst == LOAD_SYMBOL)
                            color_print_inst("LOAD_SYMBOL", Arg { ArgKind::Symbol, arg });
                        else if (inst == LOAD_CONST)
                            color_print_inst("LOAD_CONST", Arg { ArgKind::Value, arg });
                        else if (inst == POP_JUMP_IF_TRUE)
                            color_print_inst("POP_JUMP_IF_TRUE", Arg { ArgKind::Raw, arg });
                        else if (inst == STORE)
                            color_print_inst("STORE", Arg { ArgKind::Symbol, arg });
                        else if (inst == SET_VAL)
                            color_print_inst("SET_VAL", Arg { ArgKind::Symbol, arg });
                        else if (inst == POP_JUMP_IF_FALSE)
                            color_print_inst("POP_JUMP_IF_FALSE", Arg { ArgKind::Raw, arg });
                        else if (inst == JUMP)
                            color_print_inst("JUMP", Arg { ArgKind::Raw, arg });
                        else if (inst == RET)
                            color_print_inst("RET");
                        else if (inst == HALT)
                            color_print_inst("HALT");
                        else if (inst == CALL)
                            color_print_inst("CALL", Arg { ArgKind::Raw, arg });
                        else if (inst == CAPTURE)
                            color_print_inst("CAPTURE", Arg { ArgKind::Symbol, arg });
                        else if (inst == BUILTIN)
                            color_print_inst("BUILTIN", Arg { ArgKind::Builtin, arg });
                        else if (inst == DEL)
                            color_print_inst("DEL", Arg { ArgKind::Symbol, arg });
                        else if (inst == MAKE_CLOSURE)
                            color_print_inst("MAKE_CLOSURE", Arg { ArgKind::Value, arg });
                        else if (inst == GET_FIELD)
                            color_print_inst("GET_FIELD", Arg { ArgKind::Symbol, arg });
                        else if (inst == PLUGIN)
                            color_print_inst("PLUGIN", Arg { ArgKind::Value, arg });
                        else if (inst == LIST)
                            color_print_inst("LIST", Arg { ArgKind::Raw, arg });
                        else if (inst == APPEND)
                            color_print_inst("APPEND", Arg { ArgKind::Raw, arg });
                        else if (inst == CONCAT)
                            color_print_inst("CONCAT", Arg { ArgKind::Raw, arg });
                        else if (inst == APPEND_IN_PLACE)
                            color_print_inst("APPEND_IN_PLACE", Arg { ArgKind::Raw, arg });
                        else if (inst == CONCAT_IN_PLACE)
                            color_print_inst("CONCAT_IN_PLACE", Arg { ArgKind::Raw, arg });
                        else if (inst == POP_LIST)
                            color_print_inst("POP_LIST");
                        else if (inst == POP_LIST_IN_PLACE)
                            color_print_inst("POP_LIST_IN_PLACE");
                        else if (inst == POP)
                            color_print_inst("POP");
                        else if (inst == DUP)
                            color_print_inst("DUP");
                        else if (inst == ADD)
                            color_print_inst("ADD");
                        else if (inst == SUB)
                            color_print_inst("SUB");
                        else if (inst == MUL)
                            color_print_inst("MUL");
                        else if (inst == DIV)
                            color_print_inst("DIV");
                        else if (inst == GT)
                            color_print_inst("GT");
                        else if (inst == LT)
                            color_print_inst("LT");
                        else if (inst == LE)
                            color_print_inst("LE");
                        else if (inst == GE)
                            color_print_inst("GE");
                        else if (inst == NEQ)
                            color_print_inst("NEQ");
                        else if (inst == EQ)
                            color_print_inst("EQ");
                        else if (inst == LEN)
                            color_print_inst("LEN");
                        else if (inst == EMPTY)
                            color_print_inst("EMPTY");
                        else if (inst == TAIL)
                            color_print_inst("TAIL");
                        else if (inst == HEAD)
                            color_print_inst("HEAD");
                        else if (inst == ISNIL)
                            color_print_inst("ISNIL");
                        else if (inst == ASSERT)
                            color_print_inst("ASSERT");
                        else if (inst == TO_NUM)
                            color_print_inst("TO_NUM");
                        else if (inst == TO_STR)
                            color_print_inst("TO_STR");
                        else if (inst == AT)
                            color_print_inst("AT");
                        else if (inst == MOD)
                            color_print_inst("MOD");
                        else if (inst == TYPE)
                            color_print_inst("TYPE");
                        else if (inst == HASFIELD)
                            color_print_inst("HASFIELD");
                        else if (inst == NOT)
                            color_print_inst("NOT");
                        else
                            fmt::println("Unknown instruction");
                    }
                }
                if (displayCode && segment != BytecodeSegment::HeadersOnly)
                    fmt::print("\n");

                ++pp;
            }
        }
    }

    uint16_t BytecodeReader::readNumber(std::size_t& i) const
    {
        const auto x = static_cast<uint16_t>(m_bytecode[i] << 8);
        const uint16_t y = m_bytecode[++i];
        return x + y;
    }
}
