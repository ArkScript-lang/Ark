#include <Ark/Compiler/IntermediateRepresentation/IRCompiler.hpp>

#include <chrono>
#include <utility>
#include <optional>
#include <unordered_map>
#include <Proxy/Picosha2.hpp>
#include <fmt/ostream.h>

#include <Ark/Constants.hpp>
#include <Ark/Utils/Literals.hpp>
#include <Ark/Compiler/IntermediateRepresentation/InstLoc.hpp>
#include <Ark/Compiler/Serialization/IntegerSerializer.hpp>
#include <Ark/Compiler/Serialization/IEEE754Serializer.hpp>

namespace Ark::internal
{
    using namespace literals;

    IRCompiler::IRCompiler(const unsigned debug) :
        Pass("IRCompiler", debug)
    {}

    void IRCompiler::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        m_logger.traceStart("process");
        pushFileHeader();
        pushSymbolTable(symbols);
        pushValueTable(values);

        // compute a list of unique filenames
        for (const auto& page : pages)
        {
            for (const auto& inst : page.data)
            {
                if (std::ranges::find(m_filenames, inst.filename()) == m_filenames.end() && inst.hasValidSourceLocation())
                    m_filenames.push_back(inst.filename());
            }
        }

        pushFilenameTable();
        pushInstLocTable(pages);

        m_ir = pages;
        compile();

        if (m_ir.empty())
        {
            // code segment with a single instruction
            m_bytecode.push_back(CODE_SEGMENT_START);
            m_bytecode.push_back(0_u8);
            m_bytecode.push_back(1_u8);

            m_bytecode.push_back(0_u8);
            m_bytecode.push_back(HALT);
            m_bytecode.push_back(0_u8);
            m_bytecode.push_back(0_u8);
        }

        // generate a hash of the tables + bytecode
        std::vector<unsigned char> hash_out(picosha2::k_digest_size);
        picosha2::hash256(m_bytecode.begin() + bytecode::HeaderSize, m_bytecode.end(), hash_out);
        m_bytecode.insert(m_bytecode.begin() + bytecode::HeaderSize, hash_out.begin(), hash_out.end());

        m_logger.traceEnd();
    }

    void IRCompiler::dumpToStream(std::ostream& stream) const
    {
        std::size_t index = 0;
        for (const auto& block : m_ir)
        {
            if (index == 0)
                // global scope
                fmt::println(stream, "global");
            else
            {
                fmt::println(
                    stream,
                    "page_{} ({} ({} argument{}) {}, {} instructions)",
                    index,
                    block.debugName(),
                    block.metadata.argument_count,
                    block.metadata.argument_count == 1 ? "" : "s",
                    block.metadataRepr(),
                    block.data.size());
            }

            for (const auto& entity : block.data)
            {
                switch (entity.kind())
                {
                    case IR::Kind::Label:
                        fmt::println(stream, ".L{}:", entity.label());
                        break;

                    case IR::Kind::Goto:
                        fmt::println(stream, "\t{} L{}", InstructionNames[entity.inst()], entity.label());
                        break;

                    case IR::Kind::GotoWithArg:
                        fmt::println(stream, "\t{} L{}, {}", InstructionNames[entity.inst()], entity.label(), entity.primaryArg());
                        break;

                    case IR::Kind::Opcode:
                        fmt::println(stream, "\t{} {}", InstructionNames[entity.inst()], entity.primaryArg());
                        break;

                    case IR::Kind::Opcode2Args:
                        fmt::println(stream, "\t{} {}, {}", InstructionNames[entity.inst()], entity.primaryArg(), entity.secondaryArg());
                        break;

                    case IR::Kind::Opcode3Args:
                        fmt::println(stream, "\t{} {}, {}, {}", InstructionNames[entity.inst()], entity.primaryArg(), entity.secondaryArg(), entity.tertiaryArg());
                        break;
                }
            }

            fmt::println(stream, "");
            ++index;
        }
    }

    const bytecode_t& IRCompiler::bytecode() const noexcept
    {
        return m_bytecode;
    }

    void IRCompiler::compile()
    {
        // push the different code segments
        for (std::size_t i = 0, end = m_ir.size(); i < end; ++i)
        {
            IR::Block& page = m_ir[i];
            // just in case we got too far, always add a HALT to be sure the
            // VM won't do anything crazy
            page.data.emplace_back(HALT);

            // push number of elements
            const std::size_t page_size = page.instructionCount();
            if (std::cmp_greater(page_size, MaxValue16Bits))
            {
                std::string message;
                if (i == 0)
                    message = fmt::format("Global scope exceeds the maximum number of instructions ({})", MaxValue16Bits);
                else if (page.metadata.name.has_value())
                    message = fmt::format("Function {} exceeds the maximum number of instructions ({})", page.metadata.name.value(), MaxValue16Bits);
                else
                    message = fmt::format("Anonymous function at page {} exceeds the maximum number of instructions ({})", i, MaxValue16Bits);

                throw std::overflow_error(message);
            }

            m_bytecode.push_back(CODE_SEGMENT_START);
            serializeOn2BytesToVecBE(page_size, m_bytecode);

            // register labels position
            uint16_t pos = 0;
            std::unordered_map<IR::label_t, uint16_t> label_to_position;
            for (const auto& inst : page.data)
            {
                switch (inst.kind())
                {
                    case IR::Kind::Label:
                        label_to_position[inst.label()] = pos;
                        break;

                    default:
                        ++pos;
                }
            }

            for (const auto& inst : page.data)
            {
                switch (inst.kind())
                {
                    case IR::Kind::Goto:
                        pushWord(Word(inst.inst(), label_to_position[inst.label()]));
                        break;

                    case IR::Kind::GotoWithArg:
                        pushWord(Word(inst.inst(), inst.primaryArg(), label_to_position[inst.label()]));
                        break;

                    case IR::Kind::Opcode:
                        [[fallthrough]];
                    case IR::Kind::Opcode2Args:
                        [[fallthrough]];
                    case IR::Kind::Opcode3Args:
                        pushWord(inst.bytecode());
                        break;

                    default:
                        break;
                }
            }
        }
    }

    void IRCompiler::pushWord(const Word& word)
    {
        m_bytecode.push_back(word.opcode);
        m_bytecode.push_back(word.byte_1);
        m_bytecode.push_back(word.byte_2);
        m_bytecode.push_back(word.byte_3);
    }

    void IRCompiler::pushFileHeader() noexcept
    {
        /*
            Generating headers:
                - lang name (to be sure we are executing an ArkScript file)
                    on 4 bytes (ark + padding)
                - version (major: 2 bytes, minor: 2 bytes, patch: 2 bytes)
                - timestamp (8 bytes, unix format)
        */

        m_bytecode.push_back('a');
        m_bytecode.push_back('r');
        m_bytecode.push_back('k');
        m_bytecode.push_back(0_u8);

        // push version
        for (const int n : std::array { ARK_VERSION_MAJOR, ARK_VERSION_MINOR, ARK_VERSION_PATCH })
            serializeOn2BytesToVecBE(n, m_bytecode);

        // push timestamp
        const long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                                        std::chrono::system_clock::now().time_since_epoch())
                                        .count();
        for (long i = 0; i < 8; ++i)
        {
            const long shift = 8 * (7 - i);
            const auto ts_byte = static_cast<uint8_t>((timestamp & (0xffLL << shift)) >> shift);
            m_bytecode.push_back(ts_byte);
        }
    }

    void IRCompiler::pushSymbolTable(const std::vector<std::string>& symbols)
    {
        const std::size_t symbol_size = symbols.size();
        if (std::cmp_greater(symbol_size, MaxValue16Bits))
            throw std::overflow_error(fmt::format("Too many symbols: {}, exceeds the maximum size of {}", symbol_size, MaxValue16Bits));

        m_bytecode.push_back(SYM_TABLE_START);
        serializeOn2BytesToVecBE(symbol_size, m_bytecode);

        for (const auto& sym : symbols)
        {
            // push the string, null terminated
            std::ranges::transform(sym, std::back_inserter(m_bytecode), [](const char i) {
                return static_cast<uint8_t>(i);
            });
            m_bytecode.push_back(0_u8);
        }
    }

    void IRCompiler::pushValueTable(const std::vector<ValTableElem>& values)
    {
        const std::size_t value_size = values.size();
        if (std::cmp_greater(value_size, MaxValue16Bits))
            throw std::overflow_error(fmt::format("Too many values: {}, exceeds the maximum size of {}", value_size, MaxValue16Bits));

        m_bytecode.push_back(VAL_TABLE_START);
        serializeOn2BytesToVecBE(value_size, m_bytecode);

        for (const ValTableElem& val : values)
        {
            switch (val.type)
            {
                case ValTableElemType::Number:
                {
                    m_bytecode.push_back(NUMBER_TYPE);
                    const auto n = std::get<double>(val.value);
                    const auto [exponent, mantissa] = ieee754::serialize(n);
                    serializeToVecLE(exponent, m_bytecode);
                    serializeToVecLE(mantissa, m_bytecode);
                    break;
                }

                case ValTableElemType::String:
                {
                    m_bytecode.push_back(STRING_TYPE);
                    auto t = std::get<std::string>(val.value);
                    std::ranges::transform(t, std::back_inserter(m_bytecode), [](const char i) {
                        return static_cast<uint8_t>(i);
                    });
                    break;
                }

                case ValTableElemType::PageAddr:
                {
                    m_bytecode.push_back(FUNC_TYPE);
                    const std::size_t addr = std::get<std::size_t>(val.value);
                    serializeOn2BytesToVecBE(addr, m_bytecode);
                    break;
                }
            }

            m_bytecode.push_back(0_u8);
        }
    }

    void IRCompiler::pushFilenameTable()
    {
        if (std::cmp_greater(m_filenames.size(), MaxValue16Bits))
            throw std::overflow_error(fmt::format("Too many filenames: {}, exceeds the maximum size of {}", m_filenames.size(), MaxValue16Bits));

        m_bytecode.push_back(FILENAMES_TABLE_START);
        // push number of elements
        serializeOn2BytesToVecBE(m_filenames.size(), m_bytecode);

        for (const auto& name : m_filenames)
        {
            std::ranges::transform(name, std::back_inserter(m_bytecode), [](const char i) {
                return static_cast<uint8_t>(i);
            });
            m_bytecode.push_back(0_u8);
        }
    }

    void IRCompiler::pushInstLocTable(const std::vector<IR::Block>& pages)
    {
        std::vector<internal::InstLoc> locations;
        for (std::size_t i = 0, end = pages.size(); i < end; ++i)
        {
            const auto& page = pages[i];
            uint16_t ip = 0;

            for (const auto& inst : page.data)
            {
                if (inst.hasValidSourceLocation())
                {
                    // we are guaranteed to have a value since we listed all existing filenames in IRCompiler::process before,
                    // thus we do not have to check if std::ranges::find returned a valid iterator.
                    auto file_id = static_cast<uint16_t>(std::distance(m_filenames.begin(), std::ranges::find(m_filenames, inst.filename())));

                    std::optional<internal::InstLoc> prev = std::nullopt;
                    if (!locations.empty())
                        prev = locations.back();

                    // skip redundant instruction location
                    if (!(prev.has_value() && prev->filename_id == file_id && prev->line == inst.sourceLine() && prev->page_pointer == i))
                        locations.push_back(
                            { .page_pointer = static_cast<uint16_t>(i),
                              .inst_pointer = ip,
                              .filename_id = file_id,
                              .line = static_cast<uint32_t>(inst.sourceLine()) });
                }

                if (inst.kind() != IR::Kind::Label)
                    ++ip;
            }
        }

        m_bytecode.push_back(INST_LOC_TABLE_START);
        serializeOn2BytesToVecBE(locations.size(), m_bytecode);

        for (const auto& loc : locations)
        {
            serializeOn2BytesToVecBE(loc.page_pointer, m_bytecode);
            serializeOn2BytesToVecBE(loc.inst_pointer, m_bytecode);
            serializeOn2BytesToVecBE(loc.filename_id, m_bytecode);
            serializeToVecBE(loc.line, m_bytecode);
        }
    }
}
