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
        SET_VAL = 0x05,
        POP_JUMP_IF_FALSE = 0x06,
        JUMP = 0x07,
        RET = 0x08,
        HALT = 0x09,
        CALL = 0x0a,
        CAPTURE = 0x0b,
        BUILTIN = 0x0c,
        DEL = 0x0d,
        MAKE_CLOSURE = 0x0e,
        GET_FIELD = 0x0f,
        PLUGIN = 0x10,
        LIST = 0x11,
        APPEND = 0x12,
        CONCAT = 0x13,
        APPEND_IN_PLACE = 0x14,
        CONCAT_IN_PLACE = 0x15,
        POP_LIST = 0x16,
        POP_LIST_IN_PLACE = 0x17,
        POP = 0x18,
        DUP = 0x19,
        LAST_COMMAND = 0x19,

        FIRST_OPERATOR = 0x1a,
        ADD = 0x1a,
        SUB = 0x1b,
        MUL = 0x1c,
        DIV = 0x1d,
        GT = 0x1e,
        LT = 0x1f,
        LE = 0x20,
        GE = 0x21,
        NEQ = 0x22,
        EQ = 0x23,
        LEN = 0x24,
        EMPTY = 0x25,
        TAIL = 0x26,
        HEAD = 0x27,
        ISNIL = 0x28,
        ASSERT = 0x29,
        TO_NUM = 0x2a,
        TO_STR = 0x2b,
        AT = 0x2c,
        MOD = 0x2d,
        TYPE = 0x2e,
        HASFIELD = 0x2f,
        NOT = 0x30,
        LAST_OPERATOR = 0x30,

        LAST_INSTRUCTION = 0x30
    };
}

#endif
