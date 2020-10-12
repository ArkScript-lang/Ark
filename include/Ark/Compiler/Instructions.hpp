#ifndef ark_compiler_instructions
#define ark_compiler_instructions

#include <cinttypes>

namespace Ark::internal
{
    /**
     * @brief The different bytecodes are stored here
     * 
     */
    enum Instruction
    {
        NOP = 0x00,
        SYM_TABLE_START = 0x01,
        VAL_TABLE_START = 0x02,
            NUMBER_TYPE = 0x01,
            STRING_TYPE = 0x02,
            FUNC_TYPE = 0x03,
        CODE_SEGMENT_START = 0x03,

        FIRST_COMMAND = 0x01,
            LOAD_SYMBOL = 0x01,
            LOAD_CONST  = 0x02,
            POP_JUMP_IF_TRUE = 0x03,
            STORE = 0x04,
            LET   = 0x05,
            POP_JUMP_IF_FALSE = 0x06,
            JUMP = 0x07,
            RET  = 0x08,
            HALT = 0x09,
            CALL = 0x0a,
            CAPTURE = 0x0b,
            BUILTIN  = 0x0c,
            MUT = 0x0d,
            DEL = 0x0e,
            SAVE_ENV = 0x0f,
            GET_FIELD = 0x10,
            PLUGIN = 0x11,
        LAST_COMMAND = 0x11,

        // NB: when adding an operator, it must be referenced as well under
        // src/VM/Builtins/Builtins.cpp, in the operators table
        // The order of the operators below must be the same as the one in
        // the operators table from src/VM/Builtins/Builtins.cpp
        FIRST_OPERATOR = 0x20,
            ADD = 0x20,
            SUB = 0x21,
            MUL = 0x22,
            DIV = 0x23,
            GT  = 0x24,
            LT  = 0x25,
            LE  = 0x26,
            GE  = 0x27,
            NEQ = 0x28,
            EQ  = 0x29,
            LEN = 0x2a,
            EMPTY = 0x2b,
            FIRSTOF = 0x2c,
            TAILOF  = 0x2d,
            HEADOF  = 0x2e,
            ISNIL  = 0x2f,
            ASSERT = 0x30,
            TO_NUM = 0x31,
            TO_STR = 0x32,
            AT = 0x33,
            AND_ = 0x34,
            OR_  = 0x35,
            MOD  = 0x36,
            TYPE = 0x37,
            HASFIELD = 0x38,
            NOT = 0x39,
        LAST_OPERATOR = 0x39,

        LAST_INSTRUCTION = 0x39
    };

    // TODO remove Inst, not useful
    struct Inst
    {
        uint8_t inst = Instruction::NOP;

        Inst(Instruction inst);
        Inst(uint8_t inst);
    };
}

#endif