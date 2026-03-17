/**
 * @file Instructions.hpp
 * @author Lexy Plateau (lexplt.dev@gmail.com)
 * @brief The different instructions used by the compiler and virtual machine
 * @date 2020-10-27
 *
 * @copyright Copyright (c) 2020-2026
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
        // @role Does nothing, useful for padding
        NOP = 0x00,
        SYM_TABLE_START = 0xA1,
        VAL_TABLE_START = 0xA2,
        CODE_SEGMENT_START = 0xA3,
        NUMBER_TYPE = 0xF1,
        STRING_TYPE = 0xF2,
        FUNC_TYPE = 0xF3,
        FILENAMES_TABLE_START = 0xA4,
        INST_LOC_TABLE_START = 0xA5,

        // @args symbol id
        // @role Load a symbol from its ID onto the stack, as a reference unless it's already one
        LOAD_FAST = 0x01,

        // @args stack index
        // @role Load a symbol from the locals stack by its index (starting from the end of the current scope), as a reference unless it's already one
        LOAD_FAST_BY_INDEX = 0x02,

        // @args symbol id
        // @role Load a symbol from its ID onto the stack, avoiding the creation of a reference
        LOAD_SYMBOL = 0x03,

        // @args constant id
        // @role Load a constant from its ID onto the stack
        LOAD_CONST = 0x04,

        // @args absolute address to jump to
        // @role Jump to the provided address if the last value on the stack was equal to true. Remove the value from the stack no matter what it is
        POP_JUMP_IF_TRUE = 0x05,

        // @args symbol id
        // @role Take the value on top of the stack and create a variable in the current scope, named following the given symbol id (cf symbols table)
        STORE = 0x06,

        // @args symbol id
        // @role Store a value in a symbol without dereferencing it (used by functions only)
        STORE_REF = 0x07,

        // @args symbol id
        // @role Take the value on top of the stack and put it inside a variable named following the symbol id (cf symbols table), in the nearest scope. Raise an error if it couldn't find a scope where the variable exists
        SET_VAL = 0x08,

        // @args absolute address to jump to
        // @role Jump to the provided address if the last value on the stack was equal to false. Remove the value from the stack no matter what it is
        POP_JUMP_IF_FALSE = 0x09,

        // @args absolute address to jump to
        // @role Jump to the provided address
        JUMP = 0x0a,

        // @role If in a code segment other than the main one, quit it, and push the value on top of the stack to the new stack; should as well delete the current environment. Otherwise, acts as a `HALT`
        RET = 0x0b,

        // @role Stop the Virtual Machine
        HALT = 0x0c,

        // @role push pp, then ip on the stack, preparing for a call instruction
        PUSH_RETURN_ADDRESS = 0x0d,

        // @args argument count
        // @role Call function from its symbol id located on top of the stack. Take the given number of arguments from the top of stack and give them to the function (the first argument taken from the stack will be the last one of the function). The stack of the function is now composed of its arguments, from the first to the last one
        CALL = 0x0e,

        // @role Jump to the top of the current function and reset the current scope
        TAIL_CALL_SELF = 0x0f,

        // @args symbol id
        // @role Tell the Virtual Machine to capture the variable from the current environment. Main goal is to be able to handle closures, which need to save the environment in which they were created
        CAPTURE = 0x10,

        // @args symbol id
        // @role Tell the VM to use the given symbol for the next capture
        RENAME_NEXT_CAPTURE = 0x11,

        // @args builtin id
        // @role Push the corresponding builtin function object on the stack
        BUILTIN = 0x12,

        // @args symbol id
        // @role Remove a variable/constant named following the given symbol id (cf symbols table)
        DEL = 0x13,

        // @args constant id
        // @role Push a Closure with the page address pointed by the constant, along with the saved scope created by CAPTURE instruction(s)
        MAKE_CLOSURE = 0x14,

        // @args symbol id
        // @role Read the field named following the given symbol id (cf symbols table) of a `Closure` stored in TS. Pop TS and push the value of field read on the stack
        GET_FIELD = 0x15,

        // @args symbol id
        // @role Read the field named following the given symbol id (cf symbols table) of a `Closure` stored in TS. Pop TS and push the value of field read on the stack, wrapping it in its closure environment
        GET_FIELD_AS_CLOSURE = 0x16,

        // @args constant id
        // @role Load a plugin dynamically, plugin name is stored as a string in the constants table
        PLUGIN = 0x17,

        // @args number of elements
        // @role Create a list from the N elements pushed on the stack. Follows the function calling convention
        LIST = 0x18,

        // @args number of elements
        // @role Append N elements to a list (TS). Elements are stored in TS(1)..TS(N). Follows the function calling convention
        APPEND = 0x19,

        // @args number of elements
        // @role Concatenate N lists to a list (TS). Lists to concat to TS are stored in TS(1)..TS(N). Follows the function calling convention
        CONCAT = 0x1a,

        // @args number of elements
        // @role Append N elements to a reference to a list (TS), the list is being mutated in-place, no new object created. Elements are stored in TS(1)..TS(N). Follows the function calling convention
        APPEND_IN_PLACE = 0x1b,

        // @args number of elements
        // @role Concatenate N lists to a reference to a list (TS), the list is being mutated in-place, no new object created. Lists to concat to TS are stored in TS(1)..TS(N). Follows the function calling convention
        CONCAT_IN_PLACE = 0x1c,

        // @role Remove an element from a list (TS), given an index (TS1). Push a new list without the removed element to the stack
        POP_LIST = 0x1d,

        // @role Remove an element from a reference to a list (TS), given an index (TS1). The list is mutated in-place, no new object created
        POP_LIST_IN_PLACE = 0x1e,

        // @role Modify a reference to a list or string (TS) by replacing the element at TS1 (must be a number) by the value in TS2. The object is mutated in-place, no new object created
        SET_AT_INDEX = 0x1f,

        // @role Modify a reference to a list (TS) by replacing TS[TS2][TS1] by the value in TS3. TS[TS2] can be a string (if it is, TS3 must be a string). The object is mutated in-place, no new object created
        SET_AT_2_INDEX = 0x20,

        // @role Remove the top of the stack
        POP = 0x21,

        // @role Pop the top of the stack, if it's false, jump to an address
        SHORTCIRCUIT_AND = 0x22,

        // @role Pop the top of the stack, if it's true, jump to an address
        SHORTCIRCUIT_OR = 0x23,

        // @role Create a new local scope
        CREATE_SCOPE = 0x24,

        // @role Reset the current scope so that it is empty, and jump to a given location
        RESET_SCOPE_JUMP = 0x25,

        // @role Destroy the last local scope
        POP_SCOPE = 0x26,

        // @role Pop a List from the stack and a function, and call the function with the given list as arguments
        APPLY = 0x27,

        FIRST_OPERATOR = 0x28,

        // @role Pop the top of the stack, if it's true, trigger the debugger
        BREAKPOINT = 0x28,

        // @role Push `TS1 + TS`
        ADD = 0x29,

        // @role Push `TS1 - TS`
        SUB = 0x2a,

        // @role Push `TS1 * TS`
        MUL = 0x2b,

        // @role Push `TS1 / TS`
        DIV = 0x2c,

        // @role Push `TS1 > TS`
        GT = 0x2d,

        // @role Push `TS1 < TS`
        LT = 0x2e,

        // @role Push `TS1 <= TS`
        LE = 0x2f,

        // @role Push `TS1 >= TS`
        GE = 0x30,

        // @role Push `TS1 != TS`
        NEQ = 0x31,

        // @role Push `TS1 == TS`
        EQ = 0x32,

        // @role Push `len(TS)`, TS must be a list
        LEN = 0x33,

        // @role Push `empty?(TS)`, TS must be a list or string
        IS_EMPTY = 0x34,

        // @role Push `tail(TS)`, all the elements of TS except the first one. TS must be a list or string
        TAIL = 0x35,

        // @role Push `head(TS)`, the first element of TS or nil if empty. TS must be a list or string
        HEAD = 0x36,

        // @role Push true if TS is nil, false otherwise
        IS_NIL = 0x37,

        // @role Convert TS to number (must be a string)
        TO_NUM = 0x38,

        // @role Convert TS to string
        TO_STR = 0x39,

        // @role Push the value at index TS (must be a number) in TS1, which must be a list or string
        AT = 0x3a,

        // @role Push the value at index TS (must be a number), inside the list or string at index TS1 (must be a number) in the list at TS2
        AT_AT = 0x3b,

        // @role Push `TS1 % TS`
        MOD = 0x3c,

        // @role Push the type of TS as a string
        TYPE = 0x3d,

        // @role Check if TS1 is a closure field of TS. TS must be a Closure, TS1 a String
        HAS_FIELD = 0x3e,

        // @role Push `!TS`
        NOT = 0x3f,

        // @args constant id, constant id
        // @role Load two consts (`primary` then `secondary`) on the stack in one instruction
        LOAD_CONST_LOAD_CONST = 0x40,

        // @args constant id, symbol id
        // @role Load const `primary` into the symbol `secondary` (create a variable)
        LOAD_CONST_STORE = 0x41,

        // @args constant id, symbol id
        // @role Load const `primary` into the symbol `secondary` (search for the variable with the given symbol id)
        LOAD_CONST_SET_VAL = 0x42,

        // @args symbol id, symbol id
        // @role Store the value of the symbol `primary` into a new variable `secondary`
        STORE_FROM = 0x43,

        // @args symbol index, symbol id
        // @role Store the value of the symbol `primary` into a new variable `secondary`
        STORE_FROM_INDEX = 0x44,

        // @args symbol id, symbol id
        // @role Store the value of the symbol `primary` into an existing variable `secondary`
        SET_VAL_FROM = 0x45,

        // @args symbol index, symbol id
        // @role Store the value of the symbol `primary` into an existing variable `secondary`
        SET_VAL_FROM_INDEX = 0x46,

        // @args symbol id, count
        // @role Increment the variable `primary` by `count` and push its value on the stack
        INCREMENT = 0x47,

        // @args symbol index, count
        // @role Increment the variable `primary` by `count` and push its value on the stack
        INCREMENT_BY_INDEX = 0x48,

        // @args symbol id, count
        // @role Increment the variable `primary` by `count` and store its value in the given symbol id
        INCREMENT_STORE = 0x49,

        // @args symbol id, count
        // @role Decrement the variable `primary` by `count` and push its value on the stack
        DECREMENT = 0x4a,

        // @args symbol index, count
        // @role Decrement the variable `primary` by `count` and push its value on the stack
        DECREMENT_BY_INDEX = 0x4b,

        // @args symbol id, count
        // @role Decrement the variable `primary` by `count` and store its value in the given symbol id
        DECREMENT_STORE = 0x4c,

        // @args symbol id, symbol id
        // @role Load the symbol `primary`, compute its tail, store it in a new variable `secondary`
        STORE_TAIL = 0x4d,

        // @args symbol index, symbol id
        // @role Load the symbol `primary`, compute its tail, store it in a new variable `secondary`
        STORE_TAIL_BY_INDEX = 0x4e,

        // @args symbol id, symbol id
        // @role Load the symbol `primary`, compute its head, store it in a new variable `secondary`
        STORE_HEAD = 0x4f,

        // @args symbol index, symbol id
        // @role Load the symbol `primary`, compute its head, store it in a new variable `secondary`
        STORE_HEAD_BY_INDEX = 0x50,

        // @args number, symbol id
        // @role Create a list of `number` elements, and store it in a new variable `secondary`
        STORE_LIST = 0x51,

        // @args symbol id, symbol id
        // @role Load the symbol `primary`, compute its tail, store it in an existing variable `secondary`
        SET_VAL_TAIL = 0x52,

        // @args symbol index, symbol id
        // @role Load the symbol `primary`, compute its tail, store it in an existing variable `secondary`
        SET_VAL_TAIL_BY_INDEX = 0x53,

        // @args symbol id, symbol id
        // @role Load the symbol `primary`, compute its head, store it in an existing variable `secondary`
        SET_VAL_HEAD = 0x54,

        // @args symbol index, symbol id
        // @role Load the symbol `primary`, compute its head, store it in an existing variable `secondary`
        SET_VAL_HEAD_BY_INDEX = 0x55,

        // @args builtin id, argument count
        // @role Call a builtin by its id in `primary`, with `secondary` arguments. Bypass the stack size check because we do not push IP/PP since builtins calls do not alter the stack
        CALL_BUILTIN = 0x56,

        // @args builtin id, argument count
        // @role Call a builtin by its id in `primary`, with `secondary` arguments. Bypass the stack size check because we do not push IP/PP since builtins calls do not alter the stack, as well as the return address removal
        CALL_BUILTIN_WITHOUT_RETURN_ADDRESS = 0x57,

        // @args constant id, absolute address to jump to
        // @role Compare `TS < constant`, if the comparison fails, jump to the given address. Otherwise, does nothing
        LT_CONST_JUMP_IF_FALSE = 0x58,

        // @args constant id, absolute address to jump to
        // @role Compare `TS < constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
        LT_CONST_JUMP_IF_TRUE = 0x59,

        // @args symbol id, absolute address to jump to
        // @role Compare `TS < symbol`, if the comparison fails, jump to the given address. Otherwise, does nothing
        LT_SYM_JUMP_IF_FALSE = 0x5a,

        // @args constant id, absolute address to jump to
        // @role Compare `TS > constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
        GT_CONST_JUMP_IF_TRUE = 0x5b,

        // @args constant id, absolute address to jump to
        // @role Compare `TS > constant`, if the comparison fails, jump to the given address. Otherwise, does nothing
        GT_CONST_JUMP_IF_FALSE = 0x5c,

        // @args symbol id, absolute address to jump to
        // @role Compare `TS > symbol`, if the comparison fails, jump to the given address. Otherwise, does nothing
        GT_SYM_JUMP_IF_FALSE = 0x5d,

        // @args constant id, absolute address to jump to
        // @role Compare `TS == constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
        EQ_CONST_JUMP_IF_TRUE = 0x5e,

        // @args symbol index, absolute address to jump to
        // @role Compare `TS == symbol`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
        EQ_SYM_INDEX_JUMP_IF_TRUE = 0x5f,

        // @args constant id, absolute address to jump to
        // @role Compare `TS != constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
        NEQ_CONST_JUMP_IF_TRUE = 0x60,

        // @args symbol id, absolute address to jump to
        // @role Compare `TS != symbol`, if the comparison fails, jump to the given address. Otherwise, does nothing
        NEQ_SYM_JUMP_IF_FALSE = 0x61,

        // @args symbol id, argument count
        // @role Call a symbol by its id in `primary`, with `secondary` arguments
        CALL_SYMBOL = 0x62,

        // @args symbol id (function name), argument count
        // @role Call the current page with `secondary` arguments
        CALL_CURRENT_PAGE = 0x63,

        // @args symbol id, field id in symbols table
        // @role Push the field of a given symbol (which has to be a closure) on the stack
        GET_FIELD_FROM_SYMBOL = 0x64,

        // @args symbol index, field id in symbols table
        // @role Push the field of a given symbol (which has to be a closure) on the stack
        GET_FIELD_FROM_SYMBOL_INDEX = 0x65,

        // @args symbol id, symbol id2
        // @role Push symbol[symbol2]
        AT_SYM_SYM = 0x66,

        // @args symbol index, symbol index2
        // @role Push symbol[symbol2]
        AT_SYM_INDEX_SYM_INDEX = 0x67,

        // @args symbol index, constant id
        // @role Push symbol[constant]
        AT_SYM_INDEX_CONST = 0x68,

        // @args symbol id, constant id
        // @role Check that the type of symbol is the given constant, push true if so, false otherwise
        CHECK_TYPE_OF = 0x69,

        // @args symbol index, constant id
        // @role Check that the type of symbol is the given constant, push true if so, false otherwise
        CHECK_TYPE_OF_BY_INDEX = 0x6a,

        // @args symbol id, number of elements
        // @role Append N elements to a reference to a list (symbol id), the list is being mutated in-place, no new object created. Elements are stored in TS(1)..TS(N). Follows the function calling convention
        APPEND_IN_PLACE_SYM = 0x6b,

        // @args symbol index, number of elements
        // @role Append N elements to a reference to a list (symbol index), the list is being mutated in-place, no new object created. Elements are stored in TS(1)..TS(N). Follows the function calling convention
        APPEND_IN_PLACE_SYM_INDEX = 0x6c,

        // @args symbol index, symbol id
        // @role Compute the length of the list or string at symbol index, and store it in a variable (symbol id)
        STORE_LEN = 0x6d,

        // @args symbol id, absolute address to jump to
        // @role Compute the length of a symbol (list or string), and pop TS to compare it, then jump if false
        LT_LEN_SYM_JUMP_IF_FALSE = 0x6e,

        // @args symbol id, offset number
        // @role Multiply the symbol by (offset symbol - 2048), then push it to the stack
        MUL_BY = 0x6f,

        // @args symbol index, offset number
        // @role Multiply the symbol by (offset symbol - 2048), then push it to the stack
        MUL_BY_INDEX = 0x70,

        // @args symbol id, offset number
        // @role Multiply the symbol by (offset symbol - 2048), then store the result using the given symbol id
        MUL_SET_VAL = 0x71,

        // @args op1, op2, op3
        // @role Pop 3 or 4 values from the stack, and apply the ops sequentially (only ADD, SUB, MUL, and DIV are supported). Push the result to the stack. Only op3 may be NOP.
        FUSED_MATH = 0x72,

        InstructionsCount
    };

    constexpr std::array InstructionNames = {
        "NOP",
        "LOAD_FAST",
        "LOAD_FAST_BY_INDEX",
        "LOAD_SYMBOL",
        "LOAD_CONST",
        "POP_JUMP_IF_TRUE",
        "STORE",
        "STORE_REF",
        "SET_VAL",
        "POP_JUMP_IF_FALSE",
        "JUMP",
        "RET",
        "HALT",
        "PUSH_RETURN_ADDRESS",
        "CALL",
        "TAIL_CALL_SELF",
        "CAPTURE",
        "RENAME_NEXT_CAPTURE",
        "BUILTIN",
        "DEL",
        "MAKE_CLOSURE",
        "GET_FIELD",
        "GET_FIELD_AS_CLOSURE",
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
        "SHORTCIRCUIT_AND",
        "SHORTCIRCUIT_OR",
        "CREATE_SCOPE",
        "RESET_SCOPE_JUMP",
        "POP_SCOPE",
        "APPLY",
        // operators
        "BREAKPOINT",
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
        "IS_EMPTY",
        "TAIL",
        "HEAD",
        "IS_NIL",
        "TO_NUM",
        "TO_STR",
        "AT",
        "AT_AT",
        "MOD",
        "TYPE",
        "HAS_FIELD",
        "NOT",
        // super instructions
        "LOAD_CONST_LOAD_CONST",
        "LOAD_CONST_STORE",
        "LOAD_CONST_SET_VAL",
        "STORE_FROM",
        "STORE_FROM_INDEX",
        "SET_VAL_FROM",
        "SET_VAL_FROM_INDEX",
        "INCREMENT",
        "INCREMENT_BY_INDEX",
        "INCREMENT_STORE",
        "DECREMENT",
        "DECREMENT_BY_INDEX",
        "DECREMENT_STORE",
        "STORE_TAIL",
        "STORE_TAIL_BY_INDEX",
        "STORE_HEAD",
        "STORE_HEAD_BY_INDEX",
        "STORE_LIST",
        "SET_VAL_TAIL",
        "SET_VAL_TAIL_BY_INDEX",
        "SET_VAL_HEAD",
        "SET_VAL_HEAD_BY_INDEX",
        "CALL_BUILTIN",
        "CALL_BUILTIN_WITHOUT_RETURN_ADDRESS",
        "LT_CONST_JUMP_IF_FALSE",
        "LT_CONST_JUMP_IF_TRUE",
        "LT_SYM_JUMP_IF_FALSE",
        "GT_CONST_JUMP_IF_TRUE",
        "GT_CONST_JUMP_IF_FALSE",
        "GT_SYM_JUMP_IF_FALSE",
        "EQ_CONST_JUMP_IF_TRUE",
        "EQ_SYM_INDEX_JUMP_IF_TRUE",
        "NEQ_CONST_JUMP_IF_TRUE",
        "NEQ_SYM_JUMP_IF_FALSE",
        "CALL_SYMBOL",
        "CALL_CURRENT_PAGE",
        "GET_FIELD_FROM_SYMBOL",
        "GET_FIELD_FROM_SYMBOL_INDEX",
        "AT_SYM_SYM",
        "AT_SYM_INDEX_SYM_INDEX",
        "AT_SYM_INDEX_CONST",
        "CHECK_TYPE_OF",
        "CHECK_TYPE_OF_BY_INDEX",
        "APPEND_IN_PLACE_SYM",
        "APPEND_IN_PLACE_SYM_INDEX",
        "STORE_LEN",
        "LT_LEN_SYM_JUMP_IF_FALSE",
        "MUL_BY",
        "MUL_BY_INDEX",
        "MUL_SET_VAL",
        "FUSED_MATH"
    };

    static_assert(InstructionNames.size() == static_cast<std::size_t>(Instruction::InstructionsCount) && "Some instruction names appear to be missing");
}

#endif
