#include <Ark/Compiler/IntermediateRepresentation/IRCompiler.hpp>

#include <chrono>
#include <utility>
#include <unordered_map>
#include <picosha2.h>

#include <Ark/Constants.hpp>
#include <Ark/Literals.hpp>

namespace Ark::internal
{
    using namespace literals;

    IRCompiler::IRCompiler(const unsigned debug) :
        m_logger("IRCompiler", debug)
    {}

    void IRCompiler::process(const std::vector<IR::Block>& pages, const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        pushFileHeader();
        pushSymAndValTables(symbols, values);

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

        constexpr std::size_t header_size = 18;

        // generate a hash of the tables + bytecode
        std::vector<unsigned char> hash_out(picosha2::k_digest_size);
        picosha2::hash256(m_bytecode.begin() + header_size, m_bytecode.end(), hash_out);
        m_bytecode.insert(m_bytecode.begin() + header_size, hash_out.begin(), hash_out.end());
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
            page.emplace_back(HALT);

            // push number of elements
            const auto page_size = std::ranges::count_if(page, [](const auto& a) {
                return a.kind() != IR::Kind::Label;
            });
            if (std::cmp_greater(page_size, std::numeric_limits<uint16_t>::max()))
                throw std::overflow_error(fmt::format("Size of page {} exceeds the maximum size of 2^16 - 1", i));

            m_bytecode.push_back(CODE_SEGMENT_START);
            m_bytecode.push_back(static_cast<uint8_t>((page_size & 0xff00) >> 8));
            m_bytecode.push_back(static_cast<uint8_t>(page_size & 0x00ff));

            // register labels position
            uint16_t pos = 0;
            std::unordered_map<IR::label_t, uint16_t> label_to_position;
            for (auto inst : page)
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

            for (auto inst : page)
            {
                switch (inst.kind())
                {
                    case IR::Kind::Goto:
                        pushWord(Word(JUMP, label_to_position[inst.label()]));
                        break;

                    case IR::Kind::GotoIfTrue:
                        pushWord(Word(POP_JUMP_IF_TRUE, label_to_position[inst.label()]));
                        break;

                    case IR::Kind::GotoIfFalse:
                        pushWord(Word(POP_JUMP_IF_FALSE, label_to_position[inst.label()]));
                        break;

                    case IR::Kind::Opcode:
                        [[fallthrough]];
                    case IR::Kind::Opcode2Args:
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
        {
            m_bytecode.push_back(static_cast<uint8_t>((n & 0xff00) >> 8));
            m_bytecode.push_back(static_cast<uint8_t>(n & 0x00ff));
        }

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

    void IRCompiler::pushSymAndValTables(const std::vector<std::string>& symbols, const std::vector<ValTableElem>& values)
    {
        const std::size_t symbol_size = symbols.size();
        if (symbol_size > std::numeric_limits<uint16_t>::max())
            throw std::overflow_error(fmt::format("Too many symbols: {}, exceeds the maximum size of 2^16 - 1", symbol_size));

        m_bytecode.push_back(SYM_TABLE_START);
        m_bytecode.push_back(static_cast<uint8_t>((symbol_size & 0xff00) >> 8));
        m_bytecode.push_back(static_cast<uint8_t>(symbol_size & 0x00ff));

        for (const auto& sym : symbols)
        {
            // push the string, null terminated
            std::ranges::transform(sym, std::back_inserter(m_bytecode), [](const char i) {
                return static_cast<uint8_t>(i);
            });
            m_bytecode.push_back(0_u8);
        }

        const std::size_t value_size = values.size();
        if (value_size > std::numeric_limits<uint16_t>::max())
            throw std::overflow_error(fmt::format("Too many values: {}, exceeds the maximum size of 2^16 - 1", value_size));

        m_bytecode.push_back(VAL_TABLE_START);
        m_bytecode.push_back(static_cast<uint8_t>((value_size & 0xff00) >> 8));
        m_bytecode.push_back(static_cast<uint8_t>(value_size & 0x00ff));

        for (const ValTableElem& val : values)
        {
            switch (val.type)
            {
                case ValTableElemType::Number:
                {
                    m_bytecode.push_back(NUMBER_TYPE);
                    const auto n = std::get<double>(val.value);
                    std::string t = std::to_string(n);
                    std::ranges::transform(t, std::back_inserter(m_bytecode), [](const char i) {
                        return static_cast<uint8_t>(i);
                    });
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
                    m_bytecode.push_back(static_cast<uint8_t>((addr & 0xff00) >> 8));
                    m_bytecode.push_back(static_cast<uint8_t>(addr & 0x00ff));
                    break;
                }
            }

            m_bytecode.push_back(0_u8);
        }
    }
}
