# Ark bytecode

A basic file is composed of those headers:
- magic number, 6386283 ('ark'), then a zero padding
- symbols table
    - number of elements (two bytes, big endian)
    - strings, null terminated
- values table (aka constants table)
    - number of elements (two bytes, big endian)
        - type (1 byte)
            - 0x01 for number
            - 0x02 for string
            - 0x03 for function
        - value
            - number: represented in hexadecimal format (stored as a null terminated string), big endian
            - string: all the characters, plus a \0 at the end (aka null terminated string)
            - function: page number (two bytes, big endian)
- code segments (can have multiple code segments)
    - number of elements (two bytes, big endian), can be equal to 0
    - instructions

## Note on values table

The first ID isn't 0 but 3. The ID 0, 1 and 2 are reserved for `nil`, `false` and `true`.

## Note on jumps

Jumps are used to jump from a code segment to another, in case of functions. The page number is directly encoded on two bytes (big endian).

## Note on builtins

Builtins are handled with `BUILTIN id`, with `id` being the id of the builtin function object. The ids of the builtins are listed below.

| ID | Arguments(s) | Job |
| -- | ------------ | --- |
| `+` (0) | at least 2 | Sum the arguments |
| `-` (1) | at least 2 | Substract all the arguments to the first one |
| `*` (2) | at least 2 | Multiply first argument by all the others |
| `/` (3) | at least 2 | Divide first argument by all the others |
| `>` (4) | 2 | compare arguments |
| `<` (5) | 2 | compare arguments |
| `<=` (6) | 2 | compare arguments |
| `>=` (7) | 2 | compare arguments |
| `!=` (8) | 2 | compare arguments |
| `=` (9) | 2 | compare arguments |
| `len` (10) | 1 (must be a list) | Return the length of the given list |
| `empty?` (11) | 1 (must be a list) | Check if the given list is empty |
| `firstof` (12) | 1 (must be a list) | Return the first element of the given list |
| `tailof` (13) | at least 2 | Return all the given elements, except the first one |
| `append` (14) | at least 2 (first one must be a list) | Append all arguments to the first one |
| `concat` (15) | at least 2 (must be lists) | Concat all the lists into one |
| `list` (16) | at least 0 | Return a list composed of all the arguments |
| `nil?` (17) | 1 | Check if given argument is nil |
| `print` (18) | at least 0 | Print the given arguments |
| `assert` (19) | 2 | First argument must be a boolean. Raise an error, created from the message in the second argument, if the first argument is false |

## Instructions

| Code | Argument(s) | Job |
| ---- | ----------- | --- |
| `NOP` (0x00) | | Does Nothing |
| `LOAD_SYMBOL` (0x01) | symbol id (two bytes, big endian) | Load a symbol from its id onto the stack |
| `LOAD_CONST` (0x02) | constant id (two bytes, big endian) | Load a constant from its id onto the stack |
| `POP_JUMP_IF_TRUE` (0x03) | relative address to jump to (two bytes, big endian) | Jump to the provided address if the last value on the stack was equal to true. Remove the value from the stack no matter what it is |
| `STORE` (0x04) | symbol id (two bytes, big endian) | Take the value on top of the stack and put it inside a variable named following the symbol id (cf symbols table), in the nearest scope. Raise an error if it couldn't find a scope where the variable exists |
| `LET` (0x05) | symbol id (two bytes, big endian) | Take the value on top of the stack and create a variable in the current scope, named following the given symbol id (cf symbols table) |
| `POP_JUMP_IF_FALSE` (0x06) | relative address to jump to (two bytes, big endian) | Jump to the provided address if the last value on the stack was equal to false. Remove the value from the stack no matter what it is |
| `JUMP` (0x07) | absolute address to jump to (two byte, big endian) | Jump to the provided address |
| `RET` (0x08) | | If in a code segment other than the main one, quit it, and push the value on top of the stack to the new stack ; should as well delete the current environment. Otherwise, acts as a `HALT` |
| `HALT` (0x09) | | Stop the Virtual Machine |
| `CALL` (0x0a) | number of arguments when calling the function | Call function from its symbol id located on top of the stack. Take the given number of arguments from the top of stack and give them  to the function (the first argument taken from the stack will be the last one of the function). The stack of the function is now composed of its arguments, from the first to the last one |
| `NEW_ENV` (0x0b) | | Create a new environment (a new scope for variables) in the Virtual Machine |
| `BUILTIN` (0x0c) | id of builtin (two bytes, big endian) | Push the builtin function object on the stack |