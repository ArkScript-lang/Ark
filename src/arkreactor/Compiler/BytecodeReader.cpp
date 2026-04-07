#include <Ark/Compiler/BytecodeReader.hpp>

#include <Ark/Compiler/Instructions.hpp>
#include <Ark/Builtins/Builtins.hpp>

#include <unordered_map>
#include <Proxy/Picosha2.hpp>
#include <Ark/Compiler/Serialization/IEEE754Serializer.hpp>
#include <Ark/Compiler/Serialization/IntegerSerializer.hpp>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>

namespace Ark
{
    using namespace Ark::internal;

    BytecodeReader::BytecodeReader()
    {
        m_arg_kinds = {
            { LOAD_FAST, ArgKind::Symbol },
            { LOAD_FAST_BY_INDEX, ArgKind::Raw },
            { LOAD_SYMBOL, ArgKind::Symbol },
            { LOAD_CONST, ArgKind::Constant },
            { POP_JUMP_IF_TRUE, ArgKind::Raw },
            { STORE, ArgKind::Symbol },
            { STORE_REF, ArgKind::Symbol },
            { SET_VAL, ArgKind::Symbol },
            { POP_JUMP_IF_FALSE, ArgKind::Raw },
            { JUMP, ArgKind::Raw },
            { PUSH_RETURN_ADDRESS, ArgKind::RawHex },
            { CALL, ArgKind::Raw },
            { CAPTURE, ArgKind::Symbol },
            { RENAME_NEXT_CAPTURE, ArgKind::Symbol },
            { BUILTIN, ArgKind::Builtin },
            { DEL, ArgKind::Symbol },
            { MAKE_CLOSURE, ArgKind::Constant },
            { GET_FIELD, ArgKind::Symbol },
            { PLUGIN, ArgKind::Constant },
            { LIST, ArgKind::Raw },
            { APPEND, ArgKind::Raw },
            { CONCAT, ArgKind::Raw },
            { APPEND_IN_PLACE, ArgKind::Raw },
            { CONCAT_IN_PLACE, ArgKind::Raw },
            { POP_LIST, ArgKind::Raw },
            { POP_LIST_IN_PLACE, ArgKind::Raw },
            { SET_AT_INDEX, ArgKind::Raw },
            { SET_AT_2_INDEX, ArgKind::Raw },
            { RESET_SCOPE_JUMP, ArgKind::Raw },
            { LOAD_CONST_LOAD_CONST, ArgKind::ConstConst },
            { LOAD_CONST_STORE, ArgKind::ConstSym },
            { LOAD_CONST_SET_VAL, ArgKind::ConstSym },
            { STORE_FROM, ArgKind::SymSym },
            { STORE_FROM_INDEX, ArgKind::RawSym },
            { SET_VAL_FROM, ArgKind::SymSym },
            { SET_VAL_FROM_INDEX, ArgKind::RawSym },
            { INCREMENT, ArgKind::SymRaw },
            { INCREMENT_BY_INDEX, ArgKind::RawRaw },
            { INCREMENT_STORE, ArgKind::RawRaw },
            { DECREMENT, ArgKind::SymRaw },
            { DECREMENT_BY_INDEX, ArgKind::RawRaw },
            { DECREMENT_STORE, ArgKind::SymRaw },
            { STORE_TAIL, ArgKind::SymSym },
            { STORE_TAIL_BY_INDEX, ArgKind::RawSym },
            { STORE_HEAD, ArgKind::SymSym },
            { STORE_HEAD_BY_INDEX, ArgKind::RawSym },
            { STORE_LIST, ArgKind::RawSym },
            { SET_VAL_TAIL, ArgKind::SymSym },
            { SET_VAL_TAIL_BY_INDEX, ArgKind::RawSym },
            { SET_VAL_HEAD, ArgKind::SymSym },
            { SET_VAL_HEAD_BY_INDEX, ArgKind::RawSym },
            { CALL_BUILTIN, ArgKind::BuiltinRaw },
            { CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, ArgKind::BuiltinRaw },
            { LT_CONST_JUMP_IF_FALSE, ArgKind::ConstRaw },
            { LT_CONST_JUMP_IF_TRUE, ArgKind::ConstRaw },
            { LT_SYM_JUMP_IF_FALSE, ArgKind::SymRaw },
            { GT_CONST_JUMP_IF_TRUE, ArgKind::ConstRaw },
            { GT_CONST_JUMP_IF_FALSE, ArgKind::ConstRaw },
            { GT_SYM_JUMP_IF_FALSE, ArgKind::SymRaw },
            { EQ_CONST_JUMP_IF_TRUE, ArgKind::ConstRaw },
            { EQ_SYM_INDEX_JUMP_IF_TRUE, ArgKind::SymRaw },
            { NEQ_CONST_JUMP_IF_TRUE, ArgKind::ConstRaw },
            { NEQ_SYM_JUMP_IF_FALSE, ArgKind::SymRaw },
            { CALL_SYMBOL, ArgKind::SymRaw },
            { CALL_SYMBOL_BY_INDEX, ArgKind::RawRaw },
            { CALL_CURRENT_PAGE, ArgKind::SymRaw },
            { GET_FIELD_FROM_SYMBOL, ArgKind::SymSym },
            { GET_FIELD_FROM_SYMBOL_INDEX, ArgKind::RawSym },
            { AT_SYM_SYM, ArgKind::SymSym },
            { AT_SYM_INDEX_SYM_INDEX, ArgKind::RawRaw },
            { AT_SYM_INDEX_CONST, ArgKind::RawConst },
            { CHECK_TYPE_OF, ArgKind::SymConst },
            { CHECK_TYPE_OF_BY_INDEX, ArgKind::RawConst },
            { APPEND_IN_PLACE_SYM, ArgKind::SymRaw },
            { APPEND_IN_PLACE_SYM_INDEX, ArgKind::RawRaw },
            { STORE_LEN, ArgKind::RawSym },
            { LT_LEN_SYM_JUMP_IF_FALSE, ArgKind::SymRaw },
            { MUL_BY, ArgKind::RawRaw },
            { MUL_BY_INDEX, ArgKind::RawRaw },
            { MUL_SET_VAL, ArgKind::RawRaw },
            { FUSED_MATH, ArgKind::RawRawRaw }
        };
    }

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
        return m_bytecode.size() >= bytecode::Magic.size() &&
            m_bytecode[0] == bytecode::Magic[0] &&
            m_bytecode[1] == bytecode::Magic[1] &&
            m_bytecode[2] == bytecode::Magic[2] &&
            m_bytecode[3] == bytecode::Magic[3];
    }

    Version BytecodeReader::version() const
    {
        if (!checkMagic() || m_bytecode.size() < bytecode::Magic.size() + bytecode::Version.size())
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
        if (!checkMagic() || m_bytecode.size() < bytecode::HeaderSize)
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
        if (!checkMagic() || m_bytecode.size() < bytecode::HeaderSize + picosha2::k_digest_size)
            return {};

        std::vector<unsigned char> sha(picosha2::k_digest_size);
        for (std::size_t i = 0; i < picosha2::k_digest_size; ++i)
            sha[i] = m_bytecode[bytecode::HeaderSize + i];
        return sha;
    }

    Symbols BytecodeReader::symbols() const
    {
        if (!checkMagic() || m_bytecode.size() < bytecode::HeaderSize + picosha2::k_digest_size ||
            m_bytecode[bytecode::HeaderSize + picosha2::k_digest_size] != SYM_TABLE_START)
            return {};

        std::size_t i = bytecode::HeaderSize + picosha2::k_digest_size + 1;
        const uint16_t size = readNumber(i);
        i++;

        Symbols block;
        block.start = bytecode::HeaderSize + picosha2::k_digest_size;
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
                auto exp = deserializeLE<decltype(ieee754::DecomposedDouble::exponent)>(
                    m_bytecode.begin() + static_cast<std::vector<uint8_t>::difference_type>(i), m_bytecode.end());
                i += sizeof(decltype(exp));
                auto mant = deserializeLE<decltype(ieee754::DecomposedDouble::mantissa)>(
                    m_bytecode.begin() + static_cast<std::vector<uint8_t>::difference_type>(i), m_bytecode.end());
                i += sizeof(decltype(mant));

                const ieee754::DecomposedDouble d { exp, mant };
                double val = ieee754::deserialize(d);
                block.values.emplace_back(val);
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

    Filenames BytecodeReader::filenames(const Ark::Values& values) const
    {
        if (!checkMagic())
            return {};

        std::size_t i = values.end;
        if (m_bytecode[i] != FILENAMES_TABLE_START)
            return {};
        i++;

        const uint16_t size = readNumber(i);
        i++;

        Filenames block;
        block.start = values.end;
        block.filenames.reserve(size);

        for (uint16_t j = 0; j < size; ++j)
        {
            std::string val;
            while (m_bytecode[i] != 0)
                val.push_back(static_cast<char>(m_bytecode[i++]));
            block.filenames.emplace_back(val);
            i++;
        }

        block.end = i;
        return block;
    }

    InstLocations BytecodeReader::instLocations(const Ark::Filenames& filenames) const
    {
        if (!checkMagic())
            return {};

        std::size_t i = filenames.end;
        if (m_bytecode[i] != INST_LOC_TABLE_START)
            return {};
        i++;

        const uint16_t size = readNumber(i);
        i++;

        InstLocations block;
        block.start = filenames.end;
        block.locations.reserve(size);

        for (uint16_t j = 0; j < size; ++j)
        {
            auto pp = readNumber(i);
            i++;

            auto ip = readNumber(i);
            i++;

            auto file_id = readNumber(i);
            i++;

            auto line = deserializeBE<uint32_t>(
                m_bytecode.begin() + static_cast<std::vector<uint8_t>::difference_type>(i), m_bytecode.end());
            i += 4;

            block.locations.push_back(
                { .page_pointer = pp,
                  .inst_pointer = ip,
                  .filename_id = file_id,
                  .line = line });
        }

        block.end = i;
        return block;
    }

    Code BytecodeReader::code(const InstLocations& instLocations) const
    {
        if (!checkMagic())
            return {};

        std::size_t i = instLocations.end;

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

    std::optional<InstLoc> BytecodeReader::findSourceLocation(const std::vector<InstLoc>& inst_locations, const std::size_t ip, const std::size_t pp) const
    {
        std::optional<InstLoc> match = std::nullopt;

        for (const auto location : inst_locations)
        {
            if (location.page_pointer == pp && !match)
                match = location;

            // select the best match: we want to find the location that's nearest our instruction pointer,
            // but not equal to it as the IP will always be pointing to the next instruction,
            // not yet executed. Thus, the erroneous instruction is the previous one.
            if (location.page_pointer == pp && match && location.inst_pointer < ip / 4)
                match = location;

            // early exit because we won't find anything better, as inst locations are ordered by ascending (pp, ip)
            if (location.page_pointer > pp || (location.page_pointer == pp && location.inst_pointer >= ip / 4))
                break;
        }

        return match;
    }

    void BytecodeReader::display(const BytecodeSegment segment,
                                 const std::optional<uint16_t> sStart,
                                 const std::optional<uint16_t> sEnd,
                                 const std::optional<uint16_t> cPage) const
    {
        if (!checkMagic())
        {
            fmt::println("Invalid format");
            return;
        }

        if (segment == BytecodeSegment::All || segment == BytecodeSegment::HeadersOnly)
        {
            auto [major, minor, patch] = version();
            fmt::println("Version:   {}.{}.{}", major, minor, patch);
            fmt::println("Timestamp: {}", timestamp());
            fmt::print("SHA256:    ");
            for (const auto sha = sha256(); unsigned char h : sha)
                fmt::print("{:02x}", h);
            fmt::print("\n\n");
        }

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
        const auto files = filenames(vals);
        const auto inst_locs = instLocations(files);
        const auto code_block = code(inst_locs);

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
                            fmt::print(fmt::fg(fmt::color::red), "Value type not handled: {}\n", std::to_string(val.valueType()));
                            break;
                    }
                }
            }

            if (showVal)
                fmt::print("\n");
            if (segment == BytecodeSegment::Values)
                return;
        }

        // inst locs + file
        {
            std::size_t size = inst_locs.locations.size();
            std::size_t sliceSize = size;

            bool showVal = (segment == BytecodeSegment::All || segment == BytecodeSegment::InstructionLocation);
            if (showVal && sStart.has_value() && sEnd.has_value() && (sStart.value() > size || sEnd.value() > size))
                fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", size);
            else if (showVal && sStart.has_value() && sEnd.has_value())
                sliceSize = sEnd.value() - sStart.value() + 1;

            if (showVal || segment == BytecodeSegment::HeadersOnly)
                fmt::println("{} (length: {})", fmt::styled("Instruction locations table", fmt::fg(fmt::color::cyan)), sliceSize);
            if (showVal && size > 0)
                fmt::println(" PP, IP");

            for (std::size_t j = 0; j < size; ++j)
            {
                if (auto start = sStart; auto end = sEnd)
                    showVal = showVal && (j >= start.value() && j <= end.value());

                const auto& location = inst_locs.locations[j];
                if (showVal)
                    fmt::println("{:>3},{:>3} -> {}:{}", location.page_pointer, location.inst_pointer, files.filenames[location.filename_id], location.line);
            }

            if (showVal)
                fmt::print("\n");
        }

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
                else if (cPage.value_or(pp) == pp && segment != BytecodeSegment::HeadersOnly)
                {
                    if (sStart.has_value() && sEnd.has_value() && ((sStart.value() > page.size()) || (sEnd.value() > page.size())))
                    {
                        fmt::print(fmt::fg(fmt::color::red), "Slice start or end can't be greater than the segment size: {}\n", page.size());
                        return;
                    }

                    std::optional<InstLoc> previous_loc = std::nullopt;

                    for (std::size_t j = sStart.value_or(0), end = sEnd.value_or(page.size()); j < end; j += 4)
                    {
                        const uint8_t inst = page[j];
                        const uint8_t padding = page[j + 1];
                        const auto arg = static_cast<uint16_t>((page[j + 2] << 8) + page[j + 3]);

                        auto maybe_loc = findSourceLocation(inst_locs.locations, j, pp);

                        // location
                        // we want to print it only when it changed, either the file, the line, or both
                        if (maybe_loc && (!previous_loc || maybe_loc != previous_loc))
                        {
                            if (!previous_loc || previous_loc->filename_id != maybe_loc->filename_id)
                                fmt::println("{}", files.filenames[maybe_loc->filename_id]);
                            fmt::print("{:>4}", maybe_loc->line + 1);
                            previous_loc = maybe_loc;
                        }
                        else
                            fmt::print("    ");
                        // instruction number
                        fmt::print(fmt::fg(fmt::color::cyan), "{:>4x}", j / 4);
                        // padding inst arg arg
                        fmt::print(" {:02x} {:02x} {:02x} {:02x} ", inst, padding, page[j + 2], page[j + 3]);

                        printInstruction(std::cout, inst, padding, arg, syms, vals);
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

    fmt::text_style withForeColor(const fmt::color color, const bool colorize)
    {
        if (colorize)
            return fmt::fg(color);
        return {};
    }

    void BytecodeReader::printInstruction(std::ostream& os, const uint8_t inst, const uint8_t padding, const uint16_t imm_arg, const Symbols& syms, const Values& vals, const bool colorize) const
    {
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

        const auto builtin_name = [](const uint16_t idx) {
            return Builtins::builtins[idx].first;
        };
        const auto value_str = [&stringify_value, &vals](const uint16_t idx) {
            return stringify_value(vals.values[idx]);
        };
        const auto symbol_name = [&syms](const uint16_t idx) {
            return syms.symbols[idx];
        };

        if (const auto inst_idx = static_cast<std::size_t>(inst); inst_idx < InstructionNames.size())
        {
            const std::string name = InstructionNames[inst_idx];
            std::optional<Arg> arg = std::nullopt;
            if (const auto iinst = static_cast<Instruction>(inst); m_arg_kinds.contains(iinst))
                arg = Arg { m_arg_kinds.at(iinst), padding, imm_arg };

            fmt::print(os, "{}", fmt::styled(name, withForeColor(fmt::color::gold, colorize)));
            if (arg.has_value())
            {
                const auto sym_color = withForeColor(fmt::color::green, colorize);
                const auto const_color = withForeColor(fmt::color::magenta, colorize);
                const auto raw_color = withForeColor(fmt::color::red, colorize);

                switch (auto [kind, _, idx] = arg.value(); kind)
                {
                    case ArgKind::Symbol:
                        fmt::print(os, " {}\n", fmt::styled(symbol_name(idx), sym_color));
                        break;
                    case ArgKind::Constant:
                        fmt::print(os, " {}\n", fmt::styled(value_str(idx), const_color));
                        break;
                    case ArgKind::Builtin:
                        fmt::print(os, " {}\n", builtin_name(idx));
                        break;
                    case ArgKind::Raw:
                        fmt::print(os, " ({})\n", fmt::styled(idx, raw_color));
                        break;
                    case ArgKind::RawHex:
                        fmt::print(os, " ({:#x})\n", fmt::styled(idx, raw_color));
                        break;
                    case ArgKind::ConstConst:
                        fmt::print(os, " {}, {}\n", fmt::styled(value_str(arg->primary()), const_color), fmt::styled(value_str(arg->secondary()), const_color));
                        break;
                    case ArgKind::ConstSym:
                        fmt::print(os, " {}, {}\n", fmt::styled(value_str(arg->primary()), const_color), fmt::styled(symbol_name(arg->secondary()), sym_color));
                        break;
                    case ArgKind::SymConst:
                        fmt::print(os, " {}, {}\n", fmt::styled(symbol_name(arg->primary()), sym_color), fmt::styled(value_str(arg->secondary()), const_color));
                        break;
                    case ArgKind::SymSym:
                        fmt::print(os, " {}, {}\n", fmt::styled(symbol_name(arg->primary()), sym_color), fmt::styled(symbol_name(arg->secondary()), sym_color));
                        break;
                    case ArgKind::BuiltinRaw:
                        fmt::print(os, " {}, {}\n", builtin_name(arg->primary()), fmt::styled(arg->secondary(), raw_color));
                        break;
                    case ArgKind::ConstRaw:
                        fmt::print(os, " {}, {}\n", fmt::styled(value_str(arg->primary()), const_color), fmt::styled(arg->secondary(), raw_color));
                        break;
                    case ArgKind::SymRaw:
                        fmt::print(os, " {}, {}\n", fmt::styled(symbol_name(arg->primary()), sym_color), fmt::styled(arg->secondary(), raw_color));
                        break;
                    case ArgKind::RawSym:
                        fmt::print(os, " {}, {}\n", fmt::styled(arg->primary(), raw_color), fmt::styled(symbol_name(arg->secondary()), sym_color));
                        break;
                    case ArgKind::RawConst:
                        fmt::print(os, " {}, {}\n", fmt::styled(arg->primary(), raw_color), fmt::styled(value_str(arg->secondary()), const_color));
                        break;
                    case ArgKind::RawRaw:
                        fmt::print(os, " {}, {}\n", fmt::styled(arg->primary(), raw_color), fmt::styled(arg->secondary(), raw_color));
                        break;
                    case ArgKind::RawRawRaw:
                        fmt::print(os, " {}, {}, {}\n", fmt::styled(arg->padding, raw_color), fmt::styled((arg->arg & 0xff00) >> 8, raw_color), fmt::styled(arg->arg & 0x00ff, raw_color));
                        break;
                }
            }
            else
                fmt::print(os, "\n");
        }
        else
            fmt::println(os, "Unknown instruction");
    }
}
