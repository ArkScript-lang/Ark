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

#include <array>

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
        SET_AT_INDEX = 0x18,
        SET_AT_2_INDEX = 0x19,
        POP = 0x1a,
        DUP = 0x1b,
        CREATE_SCOPE = 0x1c,
        POP_SCOPE = 0x1d,

        FIRST_OPERATOR = 0x1e,
        ADD = 0x1e,
        SUB = 0x1f,
        MUL = 0x20,
        DIV = 0x21,
        GT = 0x22,
        LT = 0x23,
        LE = 0x24,
        GE = 0x25,
        NEQ = 0x26,
        EQ = 0x27,
        LEN = 0x28,
        EMPTY = 0x29,
        TAIL = 0x2a,
        HEAD = 0x2b,
        ISNIL = 0x2c,
        ASSERT = 0x2d,
        TO_NUM = 0x2e,
        TO_STR = 0x2f,
        AT = 0x30,
        MOD = 0x31,
        TYPE = 0x32,
        HASFIELD = 0x33,
        NOT = 0x34,

        LOAD_CONST_LOAD_CONST = 0x35,
        LOAD_CONST_STORE = 0x36,
        LOAD_CONST_SET_VAL = 0x37,
        STORE_FROM = 0x38,
        SET_VAL_FROM = 0x39,
        INCREMENT = 0x3a,
        DECREMENT = 0x3b,
        STORE_TAIL = 0x3c,
        STORE_HEAD = 0x3d,
        SET_VAL_TAIL = 0x3e,
        SET_VAL_HEAD = 0x3f,
        CALL_BUILTIN = 0x40
    };

    constexpr std::array InstructionNames = {
        "NOP",
        "LOAD_SYMBOL",
        "LOAD_CONST",
        "POP_JUMP_IF_TRUE",
        "STORE",
        "SET_VAL",
        "POP_JUMP_IF_FALSE",
        "JUMP",
        "RET",
        "HALT",
        "CALL",
        "CAPTURE",
        "BUILTIN",
        "DEL",
        "MAKE_CLOSURE",
        "GET_FIELD",
        "PLUGIN",
        "LIST",
        "APPEND",
        "CONCAT",
        "APPEND_IN_PLACE",
        "CONCAT_IN_PLACE",
        "POP_LIST",
        "POP_LIST_IN_PLACE",
        "SET_AT_INDEX",
        "SET_AT_2_INDEX",
        "POP",
        "DUP",
        "CREATE_SCOPE",
        "POP_SCOPE",
        // operators
        "ADD",
        "SUB",
        "MUL",
        "DIV",
        "GT",
        "LT",
        "LE",
        "GE",
        "NEQ",
        "EQ",
        "LEN",
        "EMPTY",
        "TAIL",
        "HEAD",
        "ISNIL",
        "ASSERT",
        "TO_NUM",
        "TO_STR",
        "AT",
        "MOD",
        "TYPE",
        "HASFIELD",
        "NOT",
        // super instructions
        "LOAD_CONST_LOAD_CONST",
        "LOAD_CONST_STORE",
        "LOAD_CONST_SET_VAL",
        "STORE_FROM",
        "SET_VAL_FROM",
        "INCREMENT",
        "DECREMENT",
        "STORE_TAIL",
        "STORE_HEAD",
        "SET_VAL_TAIL",
        "SET_VAL_HEAD",
        "CALL_BUILTIN"
    };
}

#endif
