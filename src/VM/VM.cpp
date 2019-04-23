#include <Ark/VM/VM.hpp>

#include <Ark/Log.hpp>

namespace Ark
{
    namespace VM
    {
        VM::VM(bool debug) :
            m_debug(debug)
        {}

        VM::~VM()
        {}

        void VM::feed(const std::string& filename)
        {
            Ark::Compiler::BytecodeReader bcr;
            bcr.feed(filename);
            m_bytecode = bcr.bytecode();

            configure();
        }

        void VM::run()
        {}

        void VM::configure()
        {
            const bytecode_t& b = m_bytecode;
            std::size_t i = 0;

            auto readNumber = [&b] (std::size_t& i) -> uint16_t {
                uint16_t x = (static_cast<uint16_t>(b[  i]) << 8) +
                              static_cast<uint16_t>(b[++i]);
                return x;
            };

            // read tables and check if bytecode is valid
            if (!(b.size() > 4 && b[i++] == 'a' && b[i++] == 'r' && b[i++] == 'k' && b[i++] == Instruction::NOP))
            {
                Ark::logger.error("[Virtual Machine] invalid format: couldn't find magic constant");
                exit(1);
            }

            if (m_debug)
                Ark::logger.info("(Virtual Machine) magic constant found: ark\\0");

            if (b[i] == Instruction::SYM_TABLE_START)
            {
                if (m_debug)
                    Ark::logger.info("(Virtual Machine) symbols table");
                
                i++;
                uint16_t size = readNumber(i);
                i++;

                if (m_debug)
                    Ark::logger.info("(Virtual Machine) length:", size);
                
                for (uint16_t j=0; j < size; ++j)
                {
                    std::string symbol = "";
                    while (b[i] != 0)
                        symbol.push_back(b[i++]);
                    i++;

                    m_symbols.push_back(symbol);

                    if (m_debug)
                        Ark::logger.info("(Virtual Machine) -", symbol);
                }
            }
            else
            {
                Ark::logger.error("[Virtual Machine] Couldn't find symbols table");
                exit(1);
            }

            if (b[i] == Instruction::VAL_TABLE_START)
            {
                if (m_debug)
                    Ark::logger.info("(Virtual Machine) constants table");
                
                i++;
                uint16_t size = readNumber(i);
                i++;

                if (m_debug)
                    Ark::logger.info("(Virtual Machine) length:", size);

                for (uint16_t j=0; j < size; ++j)
                {
                    uint8_t type = b[i];
                    i++;

                    if (type == Instruction::NUMBER_TYPE)
                    {
                        std::string val = "";
                        while (b[i] != 0)
                            val.push_back(b[i++]);
                        i++;

                        m_constants.emplace_back(HugeNumber(val));
                        
                        if (m_debug)
                            Ark::logger.info("(Virtual Machine) - (Number)", val);
                    }
                    else if (type == Instruction::STRING_TYPE)
                    {
                        std::string val = "";
                        while (b[i] != 0)
                            val.push_back(b[i++]);
                        i++;

                        m_constants.emplace_back(val);
                        
                        if (m_debug)
                            Ark::logger.info("(Virtual Machine) - (String)", val);
                    }
                    else if (type == Instruction::FUNC_TYPE)
                    {
                        uint16_t addr = readNumber(i);
                        i++;

                        m_constants.emplace_back(addr);

                        if (m_debug)
                            Ark::logger.info("(Virtual Machine) - (PageAddr)", addr);
                        
                        i++;  // skip NOP
                    }
                    else
                    {
                        if (m_debug)
                            Ark::logger.info("(Virtual Machine) Unknown value type");
                        return;
                    }
                }
            }
            else
            {
                Ark::logger.error("[Virtual Machine] Couldn't find constants table");
                exit(1);
            }
            
            while (b[i] == Instruction::CODE_SEGMENT_START)
            {
                if (m_debug)
                    Ark::logger.info("(Virtual Machine) code segment");
                
                i++;
                uint16_t size = readNumber(i);
                i++;

                if (m_debug)
                    Ark::logger.info("(Virtual Machine) length:", size);
                
                m_pages.emplace_back();

                for (uint16_t j=0; j < size; ++j)
                    m_pages.back().push_back(b[i++]);
                
                i++;
            }
        }
    }
}