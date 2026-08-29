// @role Does nothing, useful for padding
X(NOP, 0x00)

// @args symbol id
// @role Load a symbol from its ID onto the stack, as a reference unless it's already one
X(LOAD_FAST, 0x01)

// @args stack index
// @role Load a symbol from the locals stack by its index (starting from the end of the current scope), as a reference unless it's already one
X(LOAD_FAST_BY_INDEX, 0x02)

// @args symbol id
// @role Load a symbol from its ID onto the stack, avoiding the creation of a reference
X(LOAD_SYMBOL, 0x03)

// @args constant id
// @role Load a constant from its ID onto the stack
X(LOAD_CONST, 0x04)

// @args absolute address to jump to
// @role Jump to the provided address if the last value on the stack was equal to true. Remove the value from the stack no matter what it is
X(POP_JUMP_IF_TRUE, 0x05)

// @args symbol id
// @role Take the value on top of the stack and create a variable in the current scope, named following the given symbol id (cf symbols table)
X(STORE, 0x06)

// @args symbol id
// @role Store a value in a symbol without dereferencing it (used by functions only)
X(STORE_REF, 0x07)

// @args symbol id
// @role Take the value on top of the stack and put it inside a variable named following the symbol id (cf symbols table), in the nearest scope. Raise an error if it couldn't find a scope where the variable exists
X(SET_VAL, 0x08)

// @args absolute address to jump to
// @role Jump to the provided address if the last value on the stack was equal to false. Remove the value from the stack no matter what it is
X(POP_JUMP_IF_FALSE, 0x09)

// @args absolute address to jump to
// @role Jump to the provided address
X(JUMP, 0x0a)

// @role If in a code segment other than the main one, quit it, and push the value on top of the stack to the new stack; should as well delete the current environment. Otherwise, acts as a `HALT`
X(RET, 0x0b)

// @role Stop the Virtual Machine
X(HALT, 0x0c)

// @role push pp, then ip on the stack, preparing for a call instruction
X(PUSH_RETURN_ADDRESS, 0x0d)

// @args argument count
// @role Call function from its symbol id located on top of the stack. Take the given number of arguments from the top of stack and give them to the function (the first argument taken from the stack will be the last one of the function). The stack of the function is now composed of its arguments, from the first to the last one
X(CALL, 0x0e)

// @role Jump to the top of the current function and reset the current scope
X(TAIL_CALL_SELF, 0x0f)

// @args symbol id
// @role Tell the Virtual Machine to capture the variable from the current environment. Main goal is to be able to handle closures, which need to save the environment in which they were created
X(CAPTURE, 0x10)

// @args symbol id
// @role Tell the VM to use the given symbol for the next capture
X(RENAME_NEXT_CAPTURE, 0x11)

// @args builtin id
// @role Push the corresponding builtin function object on the stack
X(BUILTIN, 0x12)

// @args symbol id
// @role Remove a variable/constant named following the given symbol id (cf symbols table)
X(DEL, 0x13)

// @args constant id
// @role Push a Closure with the page address pointed by the constant, along with the saved scope created by CAPTURE instruction(s)
X(MAKE_CLOSURE, 0x14)

// @args symbol id
// @role Read the field named following the given symbol id (cf symbols table) of a `Closure` stored in TS. Pop TS and push the value of field read on the stack
X(GET_FIELD, 0x15)

// @args symbol id
// @role Read the field named following the given symbol id (cf symbols table) of a `Closure` stored in TS. Pop TS and push the value of field read on the stack, wrapping it in its closure environment
X(GET_FIELD_AS_CLOSURE, 0x16)

// @args constant id
// @role Load a plugin dynamically, plugin name is stored as a string in the constants table
X(PLUGIN, 0x17)

// @args number of elements
// @role Create a list from the N elements pushed on the stack. Follows the function calling convention
X(LIST, 0x18)

// @args number of elements
// @role Append N elements to a list (TS). Elements are stored in `TS(1)..TS(N)`. Follows the function calling convention
X(APPEND, 0x19)

// @args number of elements
// @role Concatenate N lists to a list (TS). Lists to concat to TS are stored in `TS(1)..TS(N)`. Follows the function calling convention
X(CONCAT, 0x1a)

// @args number of elements
// @role Append N elements to a reference to a list (TS), the list is being mutated in-place, no new object created. Elements are stored in `TS(1)..TS(N)`. Follows the function calling convention
X(APPEND_IN_PLACE, 0x1b)

// @args number of elements
// @role Concatenate N lists to a reference to a list (TS), the list is being mutated in-place, no new object created. Lists to concat to TS are stored in `TS(1)..TS(N)`. Follows the function calling convention
X(CONCAT_IN_PLACE, 0x1c)

// @role Remove an element from a list (TS), given an index (TS1). Push a new list without the removed element to the stack
X(POP_LIST, 0x1d)

// @role Remove an element from a reference to a list (TS), given an index (TS1). The list is mutated in-place, no new object created
X(POP_LIST_IN_PLACE, 0x1e)

// @role Modify a reference to a list or string (TS) by replacing the element at TS1 (must be a number) by the value in TS2. The object is mutated in-place, no new object created
// @args push: `1` if the new value has to be pushed to the stack, `0` otherwise
X(SET_AT_INDEX, 0x1f)

// @role Modify a reference to a list (TS) by replacing `TS[TS2][TS1]` by the value in TS3. `TS[TS2]` can be a string (if it is, TS3 must be a string). The object is mutated in-place, no new object created
// @args push: `1` if the new value has to be pushed to the stack, `0` otherwise
X(SET_AT_2_INDEX, 0x20)

// @role Remove the top of the stack
X(POP, 0x21)

// @role Pop the top of the stack, if it's `false`, jump to an address
X(SHORTCIRCUIT_AND, 0x22)

// @role Pop the top of the stack, if it's `true`, jump to an address
X(SHORTCIRCUIT_OR, 0x23)

// @role Create a new local scope
X(CREATE_SCOPE, 0x24)

// @role Reset the current scope so that it is empty, and jump to a given location
X(RESET_SCOPE_JUMP, 0x25)

// @args mode: if 1: materialise the top of the stack if it's a reference, else: just destroy the scope
// @role Destroy the last local scope
X(POP_SCOPE, 0x26)

// @role Pop a List from the stack and a function, and call the function with the given list as arguments
X(APPLY, 0x27)

// @role Pop the top of the stack, if it's `true`, trigger the debugger
X(BREAKPOINT, 0x28)

// @role Push `TS1 + TS`
X(ADD, 0x29)

// @role Push `TS1 - TS`
X(SUB, 0x2a)

// @role Push `TS1 * TS`
X(MUL, 0x2b)

// @role Push `TS1 / TS`
X(DIV, 0x2c)

// @role Push `TS1 > TS`
X(GT, 0x2d)

// @role Push `TS1 < TS`
X(LT, 0x2e)

// @role Push `TS1 <= TS`
X(LE, 0x2f)

// @role Push `TS1 >= TS`
X(GE, 0x30)

// @role Push `TS1 != TS`
X(NEQ, 0x31)

// @role Push `TS1 == TS`
X(EQ, 0x32)

// @role Push `len(TS)`, TS must be a list, dict, or string
X(LEN, 0x33)

// @role Push `empty?(TS)`, TS must be a list, dict, string or nil
X(IS_EMPTY, 0x34)

// @role Push `tail(TS)`, all the elements of TS except the first one. TS must be a list or string
X(TAIL, 0x35)

// @role Push `head(TS)`, the first element of TS or nil if empty. TS must be a list or string
X(HEAD, 0x36)

// @role Push true if TS is nil, false otherwise
X(IS_NIL, 0x37)

// @role Convert TS to number (must be a string)
X(TO_NUM, 0x38)

// @role Convert TS to string
X(TO_STR, 0x39)

// @role Push the value at index TS (must be a number) in TS1, which must be a list or string
X(AT, 0x3a)

// @role Push the value at index TS (must be a number), inside the list or string at index TS1 (must be a number) in the list at TS2
X(AT_AT, 0x3b)

// @role Push `TS1 % TS`
X(MOD, 0x3c)

// @role Push the type of TS as a string
X(TYPE, 0x3d)

// @role Check if TS1 is a closure field of TS. TS must be a Closure, TS1 a String
X(HAS_FIELD, 0x3e)

// @role Push `!TS`
X(NOT, 0x3f)

// @args constant id, constant id
// @role Load two consts (`primary` then `secondary`) on the stack in one instruction
X(LOAD_CONST_LOAD_CONST, 0x40)

// @args constant id, symbol id
// @role Load const `primary` into the symbol `secondary` (create a variable)
X(LOAD_CONST_STORE, 0x41)

// @args constant id, symbol id
// @role Load const `primary` into the symbol `secondary` (search for the variable with the given symbol id)
X(LOAD_CONST_SET_VAL, 0x42)

// @args symbol id, symbol id
// @role Store the value of the symbol `primary` into a new variable `secondary`
X(STORE_FROM, 0x43)

// @args symbol index, symbol id
// @role Store the value of the symbol `primary` into a new variable `secondary`
X(STORE_FROM_INDEX, 0x44)

// @args symbol id, symbol id
// @role Store the value of the symbol `primary` into an existing variable `secondary`
X(SET_VAL_FROM, 0x45)

// @args symbol index, symbol id
// @role Store the value of the symbol `primary` into an existing variable `secondary`
X(SET_VAL_FROM_INDEX, 0x46)

// @args symbol id, count
// @role Increment the variable `primary` by `count` and push its value on the stack
X(INCREMENT, 0x47)

// @args symbol index, count
// @role Increment the variable `primary` by `count` and push its value on the stack
X(INCREMENT_BY_INDEX, 0x48)

// @args symbol id, count
// @role Increment the variable `primary` by `count` and store its value in the given symbol id
X(INCREMENT_STORE, 0x49)

// @args symbol id, count
// @role Decrement the variable `primary` by `count` and push its value on the stack
X(DECREMENT, 0x4a)

// @args symbol index, count
// @role Decrement the variable `primary` by `count` and push its value on the stack
X(DECREMENT_BY_INDEX, 0x4b)

// @args symbol id, count
// @role Decrement the variable `primary` by `count` and store its value in the given symbol id
X(DECREMENT_STORE, 0x4c)

// @args symbol id, symbol id
// @role Load the symbol `primary`, compute its tail, store it in a new variable `secondary`
X(STORE_TAIL, 0x4d)

// @args symbol index, symbol id
// @role Load the symbol `primary`, compute its tail, store it in a new variable `secondary`
X(STORE_TAIL_BY_INDEX, 0x4e)

// @args symbol id, symbol id
// @role Load the symbol `primary`, compute its head, store it in a new variable `secondary`
X(STORE_HEAD, 0x4f)

// @args symbol index, symbol id
// @role Load the symbol `primary`, compute its head, store it in a new variable `secondary`
X(STORE_HEAD_BY_INDEX, 0x50)

// @args number, symbol id
// @role Create a list of `number` elements, and store it in a new variable `secondary`
X(STORE_LIST, 0x51)

// @args symbol id, symbol id
// @role Load the symbol `primary`, compute its tail, store it in an existing variable `secondary`
X(SET_VAL_TAIL, 0x52)

// @args symbol index, symbol id
// @role Load the symbol `primary`, compute its tail, store it in an existing variable `secondary`
X(SET_VAL_TAIL_BY_INDEX, 0x53)

// @args symbol id, symbol id
// @role Load the symbol `primary`, compute its head, store it in an existing variable `secondary`
X(SET_VAL_HEAD, 0x54)

// @args symbol index, symbol id
// @role Load the symbol `primary`, compute its head, store it in an existing variable `secondary`
X(SET_VAL_HEAD_BY_INDEX, 0x55)

// @args builtin id, argument count
// @role Call a builtin by its id in `primary`, with `secondary` arguments. Bypass the stack size check because we do not push IP/PP since builtins calls do not alter the stack
X(CALL_BUILTIN, 0x56)

// @args builtin id, argument count
// @role Call a builtin by its id in `primary`, with `secondary` arguments. Bypass the stack size check because we do not push IP/PP since builtins calls do not alter the stack, as well as the return address removal
X(CALL_BUILTIN_WITHOUT_RETURN_ADDRESS, 0x57)

// @args constant id, absolute address to jump to
// @role Compare `TS < constant`, if the comparison fails, jump to the given address. Otherwise, does nothing
X(LT_CONST_JUMP_IF_FALSE, 0x58)

// @args constant id, absolute address to jump to
// @role Compare `TS < constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
X(LT_CONST_JUMP_IF_TRUE, 0x59)

// @args symbol id, absolute address to jump to
// @role Compare `TS < symbol`, if the comparison fails, jump to the given address. Otherwise, does nothing
X(LT_SYM_JUMP_IF_FALSE, 0x5a)

// @args constant id, absolute address to jump to
// @role Compare `TS > constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
X(GT_CONST_JUMP_IF_TRUE, 0x5b)

// @args constant id, absolute address to jump to
// @role Compare `TS > constant`, if the comparison fails, jump to the given address. Otherwise, does nothing
X(GT_CONST_JUMP_IF_FALSE, 0x5c)

// @args symbol id, absolute address to jump to
// @role Compare `TS > symbol`, if the comparison fails, jump to the given address. Otherwise, does nothing
X(GT_SYM_JUMP_IF_FALSE, 0x5d)

// @args constant id, absolute address to jump to
// @role Compare `TS == constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
X(EQ_CONST_JUMP_IF_TRUE, 0x5e)

// @args symbol index, absolute address to jump to
// @role Compare `TS == symbol`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
X(EQ_SYM_INDEX_JUMP_IF_TRUE, 0x5f)

// @args constant id, absolute address to jump to
// @role Compare `TS != constant`, if the comparison succeeds, jump to the given address. Otherwise, does nothing
X(NEQ_CONST_JUMP_IF_TRUE, 0x60)

// @args symbol id, absolute address to jump to
// @role Compare `TS != symbol`, if the comparison fails, jump to the given address. Otherwise, does nothing
X(NEQ_SYM_JUMP_IF_FALSE, 0x61)

// @args symbol id, argument count
// @role Call a symbol by its id in `primary`, with `secondary` arguments
X(CALL_SYMBOL, 0x62)

// @args symbol index, argument count
// @role Call a symbol by its index in the locals in `primary`, with `secondary` arguments
X(CALL_SYMBOL_BY_INDEX, 0x63)

// @args symbol id (function name), argument count
// @role Call the current page with `secondary` arguments
X(CALL_CURRENT_PAGE, 0x64)

// @args symbol id, field id in symbols table
// @role Push the field of a given symbol (which has to be a closure) on the stack
X(GET_FIELD_FROM_SYMBOL, 0x65)

// @args symbol index, field id in symbols table
// @role Push the field of a given symbol (which has to be a closure) on the stack
X(GET_FIELD_FROM_SYMBOL_INDEX, 0x66)

// @args symbol id, symbol id2
// @role Push `symbol[symbol2]`
X(AT_SYM_SYM, 0x67)

// @args symbol index, symbol index2
// @role Push `symbol[symbol2]`
X(AT_SYM_INDEX_SYM_INDEX, 0x68)

// @args symbol index, constant id
// @role Push `symbol[constant]`
X(AT_SYM_INDEX_CONST, 0x69)

// @args symbol id, constant id
// @role Check that the type of symbol is the given constant, push `true` if so, `false` otherwise
X(CHECK_TYPE_OF, 0x6a)

// @args symbol index, constant id
// @role Check that the type of symbol is the given constant, push `true` if so, `false` otherwise
X(CHECK_TYPE_OF_BY_INDEX, 0x6b)

// @args symbol id, number of elements
// @role Append `N` elements to a reference to a list (symbol id), the list is being mutated in-place, no new object created. Elements are stored in `TS(1)..TS(N)`. Follows the function calling convention
X(APPEND_IN_PLACE_SYM, 0x6c)

// @args symbol index, number of elements
// @role Append N elements to a reference to a list (symbol index), the list is being mutated in-place, no new object created. Elements are stored in `TS(1)..TS(N)`. Follows the function calling convention
X(APPEND_IN_PLACE_SYM_INDEX, 0x6d)

// @args symbol index, symbol id
// @role Compute the length of the list or string at symbol index, and store it in a variable (symbol id)
X(STORE_LEN, 0x6e)

// @args symbol id, absolute address to jump to
// @role Compute the length of a symbol (list or string), and pop TS to compare it, then jump if `false`
X(LT_LEN_SYM_JUMP_IF_FALSE, 0x6f)

// @args symbol id, offset number
// @role Multiply the symbol by (offset symbol - 2048), then push it to the stack
X(MUL_BY, 0x70)

// @args symbol index, offset number
// @role Multiply the symbol by (offset symbol - 2048), then push it to the stack
X(MUL_BY_INDEX, 0x71)

// @args symbol id, offset number
// @role Multiply the symbol by (offset symbol - 2048), then store the result using the given symbol id
X(MUL_SET_VAL, 0x72)

// @args op1, op2, op3
// @role Pop 3 or 4 values from the stack, and apply the ops sequentially (only ADD, SUB, MUL, and DIV are supported). Push the result to the stack. Only `op3` may be `NOP`.
X(FUSED_MATH, 0x73)
