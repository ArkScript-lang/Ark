/**
 * @file Instructions.hpp
 * @author Alexandre Plateau (lexplt.dev@gmail.com)
 * @brief The different instructions used by the compiler and virtual machine
 * @version 0.1
 * @date 2020-10-27
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */

#ifndef ARK_COMPILER_INSTRUCTIONS_HPP
#define ARK_COMPILER_INSTRUCTIONS_HPP

#include <cinttypes>

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
        LAST_COMMAND = 0x19,

        FIRST_OPERATOR = 0x20,
        ADD = 0x20,
        SUB = 0x21,
        MUL = 0x22,
        DIV = 0x23,
        GT = 0x24,
        LT = 0x25,
        LE = 0x26,
        GE = 0x27,
        NEQ = 0x28,
        EQ = 0x29,
        LEN = 0x2a,
        EMPTY = 0x2b,
        TAIL = 0x2c,
        HEAD = 0x2d,
        ISNIL = 0x2e,
        ASSERT = 0x2f,
        TO_NUM = 0x30,
        TO_STR = 0x31,
        AT = 0x32,
        AND_ = 0x33,
        OR_ = 0x34,
        MOD = 0x35,
        TYPE = 0x36,
        HASFIELD = 0x37,
        NOT = 0x38,
        LAST_OPERATOR = 0x38,

        LAST_INSTRUCTION = 0x38
    };
}

#endif
