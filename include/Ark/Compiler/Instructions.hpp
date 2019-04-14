#ifndef ark_compiler_instructions
#define ark_compiler_instructions

namespace Ark
{
    namespace Compiler
    {
        enum Instruction
        {
            NOP = 0x00,
            SYM_TABLE_START = 0x01,
            VAL_TABLE_START = 0x02,
                NUMBER_TYPE = 0x01,
                STRING_TYPE = 0x02,
            CODE_SEGMENT_START = 0x03,
                LOAD_SYMBOL = 0x01,
                LOAD_CONST = 0x02,
                POP_JUMP_IF_TRUE = 0x03,
                STORE = 0x04,
                LET = 0x05,
                POP_JUMP_IF_FALSE = 0x06,
                JUMP = 0x07,
                RET = 0x08,
                HALT = 0x09,
        };

        struct Inst
        {
            Instruction inst = Instruction::NOP;
            std::size_t jump_to_page = 0;

            Inst(Instruction inst);
        };
    }
}

#endif