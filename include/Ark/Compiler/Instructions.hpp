#ifndef ark_compiler_instructions
#define ark_compiler_instructions

#include <cinttypes>

namespace Ark::internal
{
    enum Instruction
    {
        NOP = 0x00,
        SYM_TABLE_START = 0x01,
        VAL_TABLE_START = 0x02,
            NUMBER_TYPE = 0x01,
            STRING_TYPE = 0x02,
            FUNC_TYPE = 0x03,
        PLUGIN_TABLE_START = 0x03,
        CODE_SEGMENT_START = 0x04,
            LOAD_SYMBOL = 0x01,
            LOAD_CONST = 0x02,
            POP_JUMP_IF_TRUE = 0x03,
            STORE = 0x04,
            LET = 0x05,
            POP_JUMP_IF_FALSE = 0x06,
            JUMP = 0x07,
            RET = 0x08,
            HALT = 0x09,
            CALL = 0x0a,
            SAVE_ENV = 0x0b,
            BUILTIN = 0x0c,
            MUT = 0x0d,
            DEL = 0x0e
            // TODO add the operators
    };

    struct Inst
    {
        uint8_t inst = Instruction::NOP;

        Inst(Instruction inst);
        Inst(uint8_t inst);
    };
}

#endif