/**
 * @file Instructions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The different instructions used by the compiler and virtual machine
 * @version 0.1
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2024
 *
 */

#ifndef ARK_COMPILER_INSTRUCTIONS_HPP
#define ARK_COMPILER_INSTRUCTIONS_HPP

namespace Ark::internal
{
    /**
     * @brief The different bytecodes are stored here
     * @par Adding an operator
     * It must be referenced as well under include/Ark/Compiler/Common.hpp, in
     * the operators table. The order of the operators below <code>FIRST_OPERATOR</code>
     * must be the same as the one in the operators table from the aforementioned file.
     *
     */
    enum Instruction : uint8_t
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
        LOAD_CONST = 0x02,
        POP_JUMP_IF_TRUE = 0x03,
        STORE = 0x04,
        LET = 0x05,
        POP_JUMP_IF_FALSE = 0x06,
        JUMP = 0x07,
        RET = 0x08,
        HALT = 0x09,
        CALL = 0x0a,
        CAPTURE = 0x0b,
        BUILTIN = 0x0c,
        MUT = 0x0d,
        DEL = 0x0e,
        SAVE_ENV = 0x0f,
        GET_FIELD = 0x10,
        PLUGIN = 0x11,
        LIST = 0x12,
        APPEND = 0x13,
        CONCAT = 0x14,
        APPEND_IN_PLACE = 0x15,
        CONCAT_IN_PLACE = 0x16,
        POP_LIST = 0x17,
        POP_LIST_IN_PLACE = 0x18,
        POP = 0x19,
        DUP = 0x1a,
        LAST_COMMAND = 0x1a,

        FIRST_OPERATOR = 0x1b,
        ADD = 0x1b,
        SUB = 0x1c,
        MUL = 0x1d,
        DIV = 0x1e,
        GT = 0x1f,
        LT = 0x20,
        LE = 0x21,
        GE = 0x22,
        NEQ = 0x23,
        EQ = 0x24,
        LEN = 0x25,
        EMPTY = 0x26,
        TAIL = 0x27,
        HEAD = 0x28,
        ISNIL = 0x29,
        ASSERT = 0x2a,
        TO_NUM = 0x2b,
        TO_STR = 0x2c,
        AT = 0x2d,
        MOD = 0x2e,
        TYPE = 0x2f,
        HASFIELD = 0x30,
        NOT = 0x31,
        LAST_OPERATOR = 0x31,

        LAST_INSTRUCTION = 0x31
    };
}

#endif
